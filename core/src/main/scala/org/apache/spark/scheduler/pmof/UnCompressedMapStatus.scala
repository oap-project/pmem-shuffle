package org.apache.spark.scheduler.pmof

import java.nio.ByteBuffer
import java.io.{Externalizable, ObjectInput, ObjectOutput}

import scala.collection.mutable
import scala.collection.mutable.ArrayBuffer

import org.roaringbitmap.RoaringBitmap

import org.apache.spark.SparkEnv
import org.apache.spark.internal.config
import org.apache.spark.storage.BlockManagerId
import org.apache.spark.util.Utils
import org.apache.spark.scheduler.MapStatus
import org.apache.spark.internal.Logging

private[spark] class UnCompressedMapStatus(
    private[this] var loc: BlockManagerId,
    private[this] var data: Array[Byte],
    private[this] var _mapTaskId: Long)
    extends MapStatus
    with Externalizable
    with Logging {

  protected def this() = this(null, null.asInstanceOf[Array[Byte]], -1) // For deserialization only
  val step = 8

  def this(loc: BlockManagerId, uncompressedSizes: Array[Long], mapTaskId: Long) = {
    this(loc, uncompressedSizes.map(MapStatus.compressSize), mapTaskId)
  }

  override def updateLocation(newLoc: BlockManagerId): Unit = {
    loc = newLoc
  }

  override def location: BlockManagerId = loc

  override def getSizeForBlock(reduceId: Int): Long = {
    val start = reduceId * step
    UnCompressedMapStatus.bytesToLong(data.slice(start, start + step))
  }

  override def mapId: Long = _mapTaskId

  override def writeExternal(out: ObjectOutput): Unit = Utils.tryOrIOException {
    loc.writeExternal(out)
    out.writeInt(data.length)
    out.write(data)
    out.writeLong(_mapTaskId)
  }

  override def readExternal(in: ObjectInput): Unit = Utils.tryOrIOException {
    loc = BlockManagerId(in)
    val len = in.readInt()
    data = new Array[Byte](len)
    in.readFully(data)
    _mapTaskId = in.readLong()
  }
}

object UnCompressedMapStatus {
  def longToBytes(x: Long): Array[Byte] = {
    val buffer = ByteBuffer.allocate(8)
    buffer.putLong(x)
    buffer.array()
  }

  def bytesToLong(bytes: Array[Byte]): Long = {
    val buffer = ByteBuffer.allocate(8);
    buffer.put(bytes)
    buffer.flip()
    buffer.getLong()
  }
}
