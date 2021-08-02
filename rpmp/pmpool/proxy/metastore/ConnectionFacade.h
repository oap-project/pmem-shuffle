#ifndef SPARK_PMOF_CONNECTIONFACADE_H
#define SPARK_PMOF_CONNECTIONFACADE_H

#include <memory>
#include <string>

#include "redis/Redis.h"

#include "pmpool/Config.h"
#include "pmpool/RLog.h"

using namespace std;

/**
 * Facade for connection to Redis
 * 
 **/
class ConnectionFacade: public std::enable_shared_from_this<ConnectionFacade>{
public:
  ConnectionFacade(std::shared_ptr<Config> config, std::shared_ptr<RLog> log,string type);
  // Redis 
  int connect();
  // Common
  string put(string key, string value);
  string get(string key);
  int exists(string key);
  std::unordered_set<std::string> scan(string pattern);
  std::unordered_set<std::string> scanAll();
  int connect(string DBPath);
  bool isConnected();

private:
  std::shared_ptr<Config> config_;
  std::shared_ptr<RLog> log_;
  bool connected_;
  int setConnected(bool connected);
  string type_;
  string REDIS = "REDIS";
  // Redis
  shared_ptr<Redis> redis_;
};

#endif