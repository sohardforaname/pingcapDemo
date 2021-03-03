# pingcapDemo

这是一个pingcap实习应聘小作业。
## 题目

某个机器的配置为：CPU 8 cores, memory 4G, HDD 4T，这个机器上有一个 1T 的无序数据文件，格式为 (key_size, key, value_size, value)，设计一个索引结构，使得并发随机地读取每一个 key-value 的代价最小。

## 想法和设计

- 考虑到最短的k-v: 
  - key_size: 4bytes, space: 1bytes, key: 1bytes, space: 1bytes;
  - val_size: 4bytes, space: 1bytes, value: 1bytes, space: 1bytes;
  - 一共14byte。
  - 在1T的文件里面最多有78,536,544,841个k-v。

- 考虑对文件分块，分成1024块，每块1Gbytes，或者是2048块，每块512Mbytes。

- 建立简单的哈希表，维护新的键值对new_k-v：(key: uint64, value: uint64)，或者(key:uint32, value:uint32)，其中key是文件中key的哈希值，value是该k-v的文件偏移量。
  - 考虑32位系统，一个哈希表包括哈希桶和链表构成。
  - 一个链表节点指针是4bytes, value是4bytes，一共8bytes。一个桶中key是4bytes，链表头指针节点是4bytes。
  - 所以这些复合结构都是8bytes一个单元，所以如果使用桶的数量是0x80000000u，且每0x00080000个桶放到一个文件中，一共4096个桶，使用BKDR Hash算法尽可能保证均匀。
  - 平均每个文件会放19,173,961个new)k-v，一共146Mbytes。
  - 以上策略也可以调整参数获得最优情况。

- 具体的算法：
  - 每次加载1.05G文件进内存，找到最靠近1G的键值对结尾，记录这个位置作为文件分割的位置。
  - 然后对这个文件里面的键值对进行哈希，然后把该哈希桶对应的文件加载到内存中。（实现时也可以把这个块里面的数据批量哈希，然后排序，再分别写入对应的哈希表中）
  - 剩下的文件块如法炮制。
  - 查询时直接查对应的哈希桶，然后把相关key的文件偏移量找出来，读取k-v然后返回即可。

- 并发
  - TODO

