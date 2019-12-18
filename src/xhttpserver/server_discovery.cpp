#include <string>
#include <xhttpserver/server_discovery.hpp>
#include "glog/logging.h"
#include "xhttpserver/server_discovery.hpp"

namespace xhttpserver {

void ZKWatchGlobal(zhandle_t *zh, int type, int state, const char *path, void *ptr) {

  ServerDiscovery *zk_cli = static_cast<ServerDiscovery *>(ptr);
  struct String_vector strings;
  struct Stat stat;
  int ret = 0;
  /* 当前event type是会话 */
  if (type == ZOO_SESSION_EVENT) {
    if (state == ZOO_EXPIRED_SESSION_STATE) {
      LOG(INFO) << "zookeeper session expired. ";
      zk_cli->Reconnect();
    } else if (state == ZOO_CONNECTING_STATE) {
      LOG(INFO) << "zookeeper session reconnecting...";
    }
      /* 会话状态是已连接 */
    else if (state == ZOO_CONNECTED_STATE) {

      int ret = zk_cli->NodeExists(zk_cli->root_, stat);
      if (ret == ZNONODE) {
        zk_cli->CreateNode(zk_cli->root_, "root");
      } else{
        if (ret) {
          LOG(INFO) << "zookeeper Node Exists error: "<< zk_cli->ErrorStr(ret);
          exit(EXIT_FAILURE);
        }
      }
      ret = zoo_get_children(zh, zk_cli->root_.c_str(), 1, &strings);
      if (ret) {
        LOG(INFO) << "zookeeper get children error: "<< zk_cli->ErrorStr(ret);
        exit(EXIT_FAILURE);
      }
      if (zk_cli->type_ == ServerType::regist && zk_cli->session_expired_) {
        zk_cli->session_expired_ = false;
        zk_cli->CreateEphemeralSequenceNode(zk_cli->root_ + "/", zk_cli->data_);
      }
      LOG(INFO)<< "zookeeper session connected.";
    }
  }
  if (state == ZOO_CONNECTED_STATE) {
    /* 当前event type是子节点事件 */
    if (type == ZOO_CHILD_EVENT) {
      ret = zoo_get_children(zh, zk_cli->root_.c_str(), 1, &strings);
      if (ret) {
        LOG(INFO) << "zookeeper get children error: "<< zk_cli->ErrorStr(ret);
        exit(EXIT_FAILURE);
      }
      int master_status = zk_cli->NodeExists(zk_cli->master_path_, stat);
      if (master_status == ZNONODE) {
        zk_cli->master_path_ = "";
      }
      zk_cli->ServiceListStatus(strings, stat);
    }
  }
}

ServerDiscovery::ServerDiscovery(const std::string &url, const std::string &root, const std::string &data,
                                 const int &timeout, ServerStatusCallback call_back, ServerType type)
    : root_(root + "/_nodes"), url_(url), data_(data), type_(type), timeout_(timeout) {
  zhandle_ = CreatHandle();
  callback_ = std::bind(call_back, std::placeholders::_1);
}

ServerDiscovery::~ServerDiscovery() {
  zookeeper_close(zhandle_);
}

zhandle_t *ServerDiscovery::CreatHandle() {
  zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
  if (url_.empty()) return NULL;
  zhandle_t *zhandle = zookeeper_init(url_.c_str(), ZKWatchGlobal, timeout_, 0, this, 0);
  if (!zhandle) {
    fprintf(stderr, "failed when connecting to zookeeper servers...retry...\n");
    exit(EXIT_FAILURE);
  }
  return zhandle;
}

void ServerDiscovery::CreateNode(const std::string &path, const std::string &data) {
  if (zoo_state(zhandle_) == 0 || path.empty()) return;
  int ret = zoo_create(zhandle_, path.c_str(), data.c_str(), 20,
                       &ZOO_OPEN_ACL_UNSAFE, 0 /*ZOO_EPHEMERAL ZOO_SEQUENCE */,
                       NULL, 0);
  if (ret) {
    LOG(INFO) << "failed when connecting to zookeeper servers...retry..." ;
    exit(EXIT_FAILURE);
  }
}

void ServerDiscovery::CreateEphemeralSequenceNode(const std::string &path, const std::string &data) {
  if (zoo_state(zhandle_) == 0 || path.empty()) return;
  char *path_buffer = new char[kZookeeperNodeDataLength];
  int ret = zoo_create(zhandle_, path.c_str(), data.c_str(), 20,
                       &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE /*ZOO_EPHEMERAL ZOO_SEQUENCE */,
                       path_buffer, kZookeeperNodeDataLength);
  self_full_path_ = path_buffer;
  delete[] path_buffer;
  path_buffer = nullptr;

  if (ret) {
    LOG(INFO) << "zookeeper create ephemeral sequence node error: "<< ErrorStr(ret);
    exit(EXIT_FAILURE);
  }
}

int ServerDiscovery::NodeExists(const std::string &path, struct Stat &stat) {
  if (path.empty() || zhandle_ == NULL) return -1;
  int ret = zoo_exists(zhandle_, path.c_str(), 0, &stat);
  return ret;
}

void ServerDiscovery::Reconnect() {
  if (zhandle_ != NULL) {
    zookeeper_close(zhandle_);
  }
  //add register code
  zhandle_ = CreatHandle();
  session_expired_ = true;
}

void ServerDiscovery::GetValue(const std::string &path) {
  if (zhandle_ == NULL || path.empty()) return;
  struct Stat stat;
  char *data = new char[kZookeeperNodeDataLength];
  int data_length = kZookeeperNodeDataLength;
  int ret = zoo_get(zhandle_, path.c_str(), 0, data, &data_length, &stat);
  data_ = data;
  data = NULL;
  delete[] data;
  if (ret) {
    LOG(INFO) << "zookeeper get error: "<< ErrorStr(ret);
    exit(EXIT_FAILURE);
  }
}

void ServerDiscovery::ServiceListStatus(const String_vector &strings, const struct Stat &stat) {
  if (&stat == NULL) return;
  if (children_num_ > stat.numChildren) {
    server_status_.type = -1;
  } else if (children_num_ < stat.numChildren) {
    server_status_.type = 1;
  }
  children_num_ = stat.numChildren;

  if (strings.count) {
    server_status_.server_list.clear();
    std::string min_node = strings.data[0];
    server_status_.server_list.emplace_back(strings.data[0]);

    for (int i = 1; i < strings.count; i++) {
      if (strings.data[i] != NULL) {
        if (min_node > strings.data[i]) {
          min_node = "";
          min_node = strings.data[i];
        }
        server_status_.server_list.emplace_back(strings.data[i]);
      }
    }
    if (master_path_.empty()) {
      master_path_ = root_ + "/" + min_node;
      server_status_.master_path = master_path_;

      GetValue(master_path_);
      server_status_.master_data = data_;

      if (type_ && self_full_path_ == master_path_) {
        server_status_.status = 1;
        server_status_.is_master = true;
      } else {
        server_status_.status = -1;
      }
    } else {
      server_status_.status = 0;
    }
  }
  callback_(server_status_);
}

const std::string ServerDiscovery::ErrorStr(int code) {
  switch (code) {
    case ZOK:return "Everything is OK";
    case ZSYSTEMERROR:return "System error";
    case ZRUNTIMEINCONSISTENCY:return "A runtime inconsistency was found";
    case ZDATAINCONSISTENCY:return "A data inconsistency was found";
    case ZCONNECTIONLOSS:return "Connection to the server has been lost";
    case ZMARSHALLINGERROR:return "Error while marshalling or unmarshalling data";
    case ZUNIMPLEMENTED:return "Operation is unimplemented";
    case ZOPERATIONTIMEOUT:return "Operation timeout";
    case ZBADARGUMENTS:return "Invalid arguments";
    case ZINVALIDSTATE:return "Invalid zhandle state";
    case ZAPIERROR:return "Api error";
    case ZNONODE:return "Node does not exist";
    case ZNOAUTH:return "Not authenticated";
    case ZBADVERSION:return "Version conflict";
    case ZNOCHILDRENFOREPHEMERALS:return "Ephemeral nodes may not have children";
    case ZNODEEXISTS:return "The node already exists";
    case ZNOTEMPTY:return "The node has children";
    case ZSESSIONEXPIRED:return "The session has been expired by the server";
    case ZINVALIDCALLBACK:return "Invalid callback specified";
    case ZINVALIDACL:return "Invalid ACL specified";
    case ZAUTHFAILED:return "Client authentication failed";
    case ZCLOSING:return "ZooKeeper is closing";
    case ZNOTHING:return "(not error) no server responses to process";
    case ZSESSIONMOVED:return "Session moved to another server, so operation is ignored";
    default:return "unknown error";
  }
}
}