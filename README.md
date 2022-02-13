# Gsim
万有引力のシミュレーターです。太陽系の惑星のうち，7つの惑星を実装済みです。  
このプログラムはX Window Systemを用いて作成しています。
  
## 使い方
1. sim.cをコンパイルし，a.outを実行します。  
2. ウィンドウが開き惑星が表示されます。  
3. 惑星を左クリックするとその星の情報(名前，惑星ナンバー，質量，速度，座標)が表示されるウィンドウが開きます。  
4. 惑星を右クリックしたままドラッグするとその惑星を動かすことができます。また，惑星は右クリックを話した際のマウスの速度によって速度が付きます。  
5. キーボードの矢印キーを押すことで，視点を上下左右に動かすことができます。  
6. Rキーを押すと今までに動かした惑星の座標・速度，視点すべてをリセットすることができます。  
7. ESCキーを押してプログラムを終了します。  

## 実行環境 (PC)
- Intel Core i5 8500 3.0GHz
- Memory 16.0GB
- OS Windows 10 21H1  

## 実行環境 (仮想マシン)
* VirtualBox 6.1.32 r149290 (Qt5.6.2)
- Intel Core i5 8500 3.0GHz
- Memory 3.8GiB
- gcc version 9.3.0
- OS Ubuntu 20.04.3 LTS
  
## 注意事項
ソースファイルのコンパイルにはlibx11-devパッケージが必要です(ubuntu環境では`sudo apt-get install libx11-dev`でインストールできます)。  
また、X11を用いているのでコンパイルにはオプションが必要です。

特別な事情が無い限り，コンパイルはこちらのコマンドで行ってください。  
`cc sim.c Flib/FFont.c Flib/FillMask.c -I /usr/include/ -L /usr/lib/ -l X11 -lm`
