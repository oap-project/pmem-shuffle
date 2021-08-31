#include "ConnectionFacade.h"

#include <string>
#include <iostream>
#include <memory>

#include "redis/Redis.h"

using namespace std;

ConnectionFacade::ConnectionFacade(std::shared_ptr<Config> config, std::shared_ptr<RLog> log,string type){
  config_ = config;
  log_ = log;
  type_ = type;
}

string ConnectionFacade::put(string key, string value){
  return redis_->set(key, value);
}

string ConnectionFacade::get(string key){
  return redis_->get(key);
}

int ConnectionFacade::exists(string key){
  return redis_->exists(key);
}

std::unordered_set<std::string> ConnectionFacade::scanAll(){
  return redis_->scanAll();
}

std::unordered_set<std::string> ConnectionFacade::scan(std::string pattern){
  return redis_->scan(pattern);
}

//Redis
int ConnectionFacade::connect(){
  redis_ = make_shared<Redis>(config_, log_);
  redis_ -> connect();
  setConnected(true);
  return 0;
}

//Common
bool ConnectionFacade::isConnected(){
  return connected_;
}

int ConnectionFacade::setConnected(bool connected){
  connected_ = connected;
  return 0;
}