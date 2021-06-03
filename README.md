# 概要
高専のときの実習講義で作成したものである。与えられた課題は「[Spider-Mouse](http://shock-a-thon.org/2014/files/output/140706_shocktahon_SPIDAR-mouse.pdf)を用いた力覚デバイスを作成しろ」というものである。また基本基盤等以外の材料は「モータ4つ」「タコ糸」であり、他のものは3Dプリンターを用いて作成できるものであれば使用可能であった。
以上を元に5人のグループワークを行い、私達の班は「任意のWaveファイル(曲)に対しリズムを出力するデバイス」の作成を試みた。

# 役割
企画・基盤やフレームの作成からプログラムまで、材料の調達以外は全て私達で行った。
その中で特に私はプログラムを担当した。

# 開発環境等
Visual Studioを利用してコーディング＆exeファイル作成を行った。 Windows環境下でRelease/Spider_Rythum.exeを起動すれば動作するはずである。
なお作成したプログラムはSpider_Rythum/MyProgramである。

# プログラムの詳細
離散フーリエ変換を行うことで周波数成分ごとに分け、それぞれの周波数に重みをもたせて差分演算等を行うことにより、リズムを抽出した(詳細は[報告書](https://github.com/mugitti9/SpiderMouse_Rhythm/blob/master/報告書.pdf)に記載)。
また、マウスの位置に応じて曲のテンポを変更できるようにした。それに応じてスパイダーマウスのテンポも同時に変更される。

# 成果物
![3Dプリンタモデル画像](https://user-images.githubusercontent.com/33088346/120671669-952c1a00-c4cc-11eb-9b2d-bb61185a2b88.jpg)
![実物画像](https://user-images.githubusercontent.com/33088346/120671676-96f5dd80-c4cc-11eb-8069-cc5775b2dac3.png)
![パソコンを踏まえた画像](https://user-images.githubusercontent.com/33088346/120671681-98270a80-c4cc-11eb-928d-ce4ce98a3eb9.png)

作成したものの

# その他詳細
実際の報告書(名前にマスクあり)は[報告書](https://github.com/mugitti9/SpiderMouse_Rhythm/blob/master/報告書.pdf)である。
