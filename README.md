# linux-load-file
Linuxでファイルを物理アドレスにロードするサンプルコード。

## usage
```
ldf.elf [OPTIONS] ADDRESS FILE

Load file to memory.

ADDRESS            Physical Address
FILE               Image file

OPTIONS
-h, --help         Show this message
```

## 使用例

物理アドレスとバイナリファイルを指定します。

```
# ldf.elf 0x730000000 file.bin
```



