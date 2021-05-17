package org.apache.spark.shuffle.pmof

import org.apache.spark._
import org.apache.spark.internal.Logging
import org.apache.spark.shuffle.IndexShuffleBlockResolver
import org.apache.spark.storage.{BlockId, BlockManager, ShuffleBlockId}
import org.apache.spark.storage.pmof.{PersistentMemoryHandler, PmemBlockOutputStream }
import org.apache.spark.network.buffer.ManagedBuffer

/**
* The mapping between logic block and physical file location. 
*
**/
private[spark] class PmemShuffleBlockResolver(
    conf: SparkConf,
    _blockManager: BlockManager = null)
  extends IndexShuffleBlockResolver(conf, _blockManager) with Logging {
  
  var partitionBufferArray: Array[PmemBlockOutputStream] = _

   /**
   * Retrieve the data for the specified block. 
   *
   */
  override def getBlockData(blockId: BlockId, dirs: Option[Array[String]]): ManagedBuffer = {
    // return BlockId corresponding ManagedBuffer
    val persistentMemoryHandler = PersistentMemoryHandler.getPersistentMemoryHandler
    persistentMemoryHandler.getPartitionManagedBuffer(blockId.name)
  }

  override def stop() {
    PersistentMemoryHandler.stop()
    super.stop()
  }

  /**
  * Remove block from persistent memory based on block id
  **/
  override def removeDataByMap(shuffleId: ShuffleId, mapId: Long): Unit ={
    val partitionNumber = conf.get("spark.sql.shuffle.partitions")
    val persistentMemoryHandler = PersistentMemoryHandler.getPersistentMemoryHandler

    for (i <- 0 to partitionNumber.toInt){
      val key = "shuffle_" + shuffleId + "_" + mapId + "_" + i
      persistentMemoryHandler.removeBlock(key)
    }
  }
}
