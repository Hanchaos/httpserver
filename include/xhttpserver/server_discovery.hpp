#ifndef ZKCLI_H
#define ZKCLI_H

#include <iostream>
#include <functional>
#include <vector>
#include "zookeeper/zookeeper.h"

namespace xhttpserver {

typedef struct {
  int type;// -1:lost; 1: add
  int status; // -1: master->slave; 0: nochange; 1: slave->master
  std::vector<std::string> server_list;
  std::string master_path;
  std::string master_data;
  bool is_master = false;
} ServerStatus;

typedef enum {
  standby = 0,
  regist = 1,
} ServerType;

using ServerStatusCallback = std::function<void(ServerStatus &status)>;

const int kZookeeperNodeDataLength = 50;

class ServerDiscovery {
 public:
/*  服务发现构造函数
* 	参数1：根节点
* 	参数2：host地址
* 	参数3：服务注册节点路径
* 	参数4：服务注册节点 IP地址与接口信息
* 	参数5：回调函数 格式为std::function<void(ServerStatus &status)>;
* 	参数6：节点工作模式：0 standby模式；1 register 模式
* */
  ServerDiscovery(const std::string &url, const std::string &root, const std::string &data,
                  const int &timeout, ServerStatusCallback call_back, ServerType type);

  ~ServerDiscovery();

 public:

 private:
  zhandle_t *CreatHandle();
  void CreateNode(const std::string &path, const std::string &data);
  void CreateEphemeralSequenceNode(const std::string &path, const std::string &data);

 private:
  bool type_;
  bool session_expired_ = true;

  int timeout_;
  struct Stat stat;

  std::string url_;
  std::string root_;
  std::string data_;

  std::string self_full_path_;
  std::string master_path_;

  zhandle_t *zhandle_;
  int32_t children_num_ = -1;
  ServerStatus server_status_;
  ServerStatusCallback callback_;

  void Reconnect();
  int NodeExists(const std::string &path,struct Stat &stat);
  void GetValue(const std::string &path);
  void ServiceListStatus(const String_vector &strings, const struct Stat &stat);
  friend void ZKWatchGlobal(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx);

  const std::string ErrorStr(int code);
};
}
#endif