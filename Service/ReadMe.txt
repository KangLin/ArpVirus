ArpVirus 工程包含 Arp 攻击库。主要功能：
攻击局域网内s.txt 与 all.txt 交集与all.txt没有的合集中的目标主机。
s.txt:要攻击的主机MAC地址列表
all.txt:已知的局域网内主机MAC地址列表

Flood 工程为TCP SYN FLOOD攻击
它有一个配置文件：config.ini
[section]
src_ip=192.168.1.253
src_mac=00219718BC69
dst_ip = 61.187.204.17
thread_number=20

//以下两值用于调整CPU使用率
sleep_time=5 //连续运行后休息时间
run_time=500 //攻击连续运行时间

