N-Cipher encode/decode data and print to standard output.

## Requirements

* libncipher >= 2.1.0

## Usage

```shellsession
% bin2cipher -i /bin/uname -o encoded.txt
% head -c 126 encoded.txt
ゃににん〜んぱす〜ぱにゃ〜んすに〜ん〜ゃ〜ゃ〜〜〜〜〜〜〜〜〜〜ん〜〜んんん〜〜ゃ〜
% cipher2bin -i encoded.txt -o decoded.bin
% cmp /bin/uname decoded.bin    # $? == 0
% chmod +x ./decoded.bin
% ./decoded.bin -a
Linux Autobahn 4.11.0-amd64-mykern1-lowlatency #1 SMP PREEMPT Thu May 4 20:15:18 JST 2017 x86_64 GNU/Linu
% bin2cipher -s 'くそぅ' -m '！' -i /bin/ls | cipher2bin -s 'くそぅ' -m '！' -o decoded.bin
% cmp /bin/ls decoded.bin       # $? == 0
% ./decoded.bin / | head
bin
boot
dev
emul
etc
home
initrd.img
lib
lib64
lost+found
```
