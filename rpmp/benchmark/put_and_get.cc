#include <string.h>
#include <cstdlib>
#include <thread>  // NOLINT
#include "Config.h"
#include "pmpool/Base.h"
#include "pmpool/client/PmPoolClient.h"

uint64_t timestamp_now() {
  return std::chrono::high_resolution_clock::now().time_since_epoch() /
    std::chrono::milliseconds(1);
}

/**
 *  One request will put 1024*1024, namely 1MB data to RPMP, the total data put to PRMP = char_size * numReqs, take char_size as 1024 * 1024,
 *  numReqs as 2048, the total data equals 2GB.
 **/
const int char_size = 1024 * 1024;
char str[char_size];
int numReqs = 2048;

bool comp(char* str, char* str_read, uint64_t size) {
  auto res = memcmp(str, str_read, size);
  if (res != 0) {
    fprintf(stderr,
        "** strcmp is %d, read res is not aligned with wrote. **\nreaded "
        "content is \n",
        res);
    for (int i = 0; i < 100; i++) {
      fprintf(stderr, "%X ", *(str_read + i));
    }
    fprintf(stderr, " ...\nwrote content is \n");
    for (int i = 0; i < 100; i++) {
      fprintf(stderr, "%X ", *(str + i));
    }
    fprintf(stderr, " ...\n");
  }
  return res == 0;
}

/**
 * The number of get operations in one thread is ${num_reqs} / ${threads}
 * 
 */
void get(int map_id, int start, int end, std::shared_ptr<PmPoolClient> client) {
  int count = start;
  while (count < end) {
    std::string key =
      "block_" + std::to_string(map_id) + "_" + std::to_string(count++);
    char str_read[char_size];
    client->begin_tx();
    client->get(key, str_read, char_size);
    client->end_tx();
    if (comp(str, str_read, char_size) == false) {
      throw;
    }
  }
}

/**
 * The number of put operations in one thread is ${num_reqs} / ${threads}
 *  
 **/
void put(int map_id, int start, int end, std::shared_ptr<PmPoolClient> client) {
  int count = start;
  while (count < end) {
    std::string key =
      "block_" + std::to_string(map_id) + "_" + std::to_string(count++);
    client->begin_tx();
    client->put(key, str, char_size);
    client->end_tx();
  }
}

int main(int argc, char** argv) {
  /// initialize Config class
  std::shared_ptr<Config> config = std::make_shared<Config>();
  CHK_ERR("config init", config->init(argc, argv));

  char temp[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
    'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f'};
  for (int i = 0; i < char_size/ 32; i++) {
    memcpy(str + i * 32, temp, 32);
  }

  /**
  for (int i = 0; i < sizeof(str); i++){
    std::cout<<str[i]<<std::endl; 
  }
  **/

  int threads = config->get_num_threads();
  int map_id = config->get_map_id();
  numReqs = config->get_num_reqs();
  std::string proxy_addrs = config->get_proxy_addrs();
  std::string proxy_port = config->get_proxy_port();

  std::cout << "=================== Put and Get ======================="
    << std::endl;
  std::cout << "RPMP proxy address(es): " << proxy_addrs << std::endl;
  std::cout << "RPMP proxy port: " << proxy_port << std::endl;
  std::cout << "Total Num Requests is " << numReqs << std::endl;
  std::cout << "Total Num Threads is " << threads << std::endl;
  std::cout << "Block key pattern is "
    << "block_" << map_id << "_*" << std::endl;

  auto client = std::make_shared<PmPoolClient>(proxy_addrs, proxy_port);
  client->init();
  std::cout << "start put." << std::endl;
  int start = 0;
  int step = numReqs / threads;
  std::vector<std::shared_ptr<std::thread>> put_threads;
  uint64_t begin = timestamp_now();
  for (int i = 0; i < threads; i++) {
    auto t =
      std::make_shared<std::thread>(put, map_id, start, start + step, client);
    put_threads.push_back(t);
    start += step;
  }
  for (auto thread : put_threads) {
    thread->join();
  }
  uint64_t end = timestamp_now();
  std::cout << "[block_" << map_id << "_*]"
    << "pmemkv put test:  " << std::to_string(char_size) <<" "
    << " bytes test, consumes " << (end - begin) / 1000.0
    << "s, throughput is " << numReqs / ((end - begin) / 1000.0)
    << "MB/s" << std::endl;

  std::cout << "start get." << std::endl;
  std::vector<std::shared_ptr<std::thread>> get_threads;
  begin = timestamp_now();
  start = 0;
  for (int i = 0; i < threads; i++) {
    auto t =
      std::make_shared<std::thread>(get, map_id, start, start + step, client);
    get_threads.push_back(t);
    start += step;
  }
  for (auto thread : get_threads) {
    thread->join();
  }
  end = timestamp_now();
  std::cout << "[block_" << map_id << "_*]"
    << "pmemkv get test: " << std::to_string(char_size) << " "
    << " bytes test, consumes " << (end - begin) / 1000.0
    << "s, throughput is " << numReqs / ((end - begin) / 1000.0)
    << "MB/s" << std::endl;

  client.reset();
  return 0;
}
