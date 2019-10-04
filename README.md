# Information Security Class

### 組員
+ 四資工三甲 B10615031 許晉捷
+ 四資工三甲 B10615053 劉彥麟

<hr />

### HW 1

##### Encrypt.cpp by 四資工三甲 B10615031 許晉捷

##### <a target="_blank" href="https://github.com/Rekkursion/Information_Security_Class/releases">exe 檔在這邊</a>

+ 使用語言：C++

+ 開發環境：Windows 10, Microsoft Visual Studio 2019

+ 指令介紹：
	* ./Encrypt.exe <strong>caesar</strong> &lt;KEY&gt; &lt;plaint text&gt;
    	- KEY 必須為一小於 2^31 的整數。
    	- 如果 KEY 為負數，可以往反方向平移，如 KEY = -2，則 abzy -> YZXW。
    * ./Encrypt.exe <strong>playfair</strong> &lt;KEY&gt; &lt;plaint text&gt;
        - KEY 必須為一只包含英文字母的字串。
        - 依照投影片的作法，將 I/J 放在同一格。
        - 兩兩分組時，若有相同字母分到同一組，將會插一個 X 在中間；若總數為奇數個，則將在最後面補上一個 X。如明文為 hell，則 HELL -> HELXL（因為 LL 在同一組） -> HELXLX（因為插完後總數變奇數個）。
    * ./Encrypt.exe <strong>vernam</strong> &lt;KEY&gt; &lt;plaint text&gt;
        - KEY 必須為一只包含英文字母的字串。
        - 若 KEY 的長度比 plain text 短，使用 Auto key 的方式補滿。
        - 將字母轉成 0-25，然後各個進行 XOR 運算，得出來的數字再加上 65；通常可以轉回大寫英文字母，不過也有可能出現其他符號。
    * ./Encrypt.exe <strong>row</strong> &lt;KEY&gt; &lt;plaint text&gt;
        - KEY 必須為一只包含數字 0-9 的字串。
        - 雖然投影片中只有 1-9，但為了保險起見，這支程式可以接受 0-9，0 比 1 優先。
        - 實作時，並沒有真的建立矩陣，而是先確定順序，然後跳著取字元即可。如 KEY 為 2341，則可以確定第一輪要取第四個 col、第二輪要取第一個 col &#8230;；取的時候第一輪從 plain text 中從第四個字元開始每四個取一次、第二輪從第一個字元開始每四個取一次&#8230;。
    * ./Encrypt.exe <strong>rail_fence</strong> &lt;KEY&gt; &lt;plaint text&gt;
        - KEY 必須為一小於 2^31 的正整數。

##### Decrypt.cpp by 四資工三甲 B10615053 劉彥麟

+ 使用語言：C++

+ 開發環境：Windows 10, Microsoft Visual Studio 2019

+ 指令介紹：
	* ./Decrypt.exe <strong>caesar</strong> &lt;KEY&gt; &lt;CIPHER TEXT&gt;
		- KEY 必須為一小於 2^31 的整數。
		- 如果 KEY 為負數，可以往反方向平移，如 KEY = -2，則 YZXW -> abzy。
	* ./Decrypt.exe <strong>row</strong> &lt;KEY&gt; &lt;CIPHER TEXT&gt;
		- KEY 必須為一只包含數字 0-9 的字串。
		- 雖然投影片中只有 1-9，但為了保險起見，這支程式可以接受 0-9，0 比 1 優先。
		- 實作時，先分別取出每個column的長度，之後利用陣列儲存每個column中第一個row在密文中的位址，按照key中的順序將整個句子重組。
	* ./Decrypt.exe <strong>rail_fence</strong> &lt;KEY&gt; &lt;CIPHER TEXT&gt;
		- KEY 必須為一小於 2^31 的正整數。
		- 實作時，利用rail fence的重複圖形來切割並計算每個row的總字數，之後便可切個密文，按照fence的排列順序重組出原文。
	
