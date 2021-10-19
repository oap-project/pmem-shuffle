/*
 * Filename: /mnt/spark-pmof/tool/rpmp/pmpool/client/PmPoolClient.h
 * Path: /mnt/spark-pmof/tool/rpmp/pmpool/client
 * Created Date: Friday, December 13th 2019, 3:43:04 pm
 * Author: root
 *
 * Copyright (c) 2019 Intel
 */

#ifndef PMPOOL_CLIENT_PMPOOLCLIENT_H_
#define PMPOOL_CLIENT_PMPOOLCLIENT_H_

#define INITIAL_BUFFER_NUMBER 64

#include <HPNL/Callback.h>
#include <HPNL/ChunkMgr.h>
#include <HPNL/Client.h>
#include <HPNL/Connection.h>

#include <atomic>
#include <condition_variable>  // NOLINT
#include <functional>
#include <future>  // NOLINT
#include <iostream>
#include <memory>
#include <mutex>  // NOLINT
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <map>
#include <unordered_set>

#include "pmpool/Base.h"
#include "pmpool/Common.h"
#include "pmpool/ThreadWrapper.h"
#include "pmpool/proxy/PhysicalNode.h"

class NetworkClient;
class RequestHandler;
class Function;
class ProxyClient;
class ProxyRequestHandler;

using std::atomic;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;

struct Channel{
  std::shared_ptr<NetworkClient> networkClient;
  std::shared_ptr<RequestHandler> requestHandler;
};

struct NodeInfo {
  std::string ip;
  std::string port;
};

class PmPoolClient {
 public:
  PmPoolClient(const string &proxy_address, const string &proxy_port);
  ~PmPoolClient();
  int init();

  /// memory pool interface
  void begin_tx();
  void end_tx();

  /// key-value storage interface
  uint64_t put(const string &key, const char *value, uint64_t size);
  uint64_t get(const string &key, char *value, uint64_t size);
  vector<block_meta> getMeta(const string &key);
  int del(const string &key);

  void shutdown();
  void wait();
  std::shared_ptr<Channel> getChannel(PhysicalNode node);

 private:
  shared_ptr<ProxyRequestHandler> proxyRequestHandler_;
  shared_ptr<ProxyClient> proxyClient_;
  atomic<uint64_t> rid_ = {0};
  std::mutex tx_mtx;
  std::condition_variable tx_con;
  bool tx_finished;
  bool op_finished;
  std::map<string, std::shared_ptr<Channel>> channels;
  std::mutex channel_mtx;
  std::unordered_set<std::string> deadNodes;
};

#endif  // PMPOOL_CLIENT_PMPOOLCLIENT_H_
