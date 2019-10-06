使用程式前需先使用指令make生成 shell_pipe

輸入./shell_pipe 即可執行此shell程式
這個程式基本功能和作業八shell_sigaction相同
額外功能多了可以使用"|"連接兩個指令。(能力不足，所以只能兩個)
當然指令只輸入一個也是可以的，但是就不能加上"|"

如果這個使用"|"連結起來的程式執行太久，可以使用ctrl+c砍掉整個行程
因為我自己測試使用ls -R / | wc 顯示permission denied (在bash下也是)
所以推薦使用ls -R / | grep proc 即可正常運作，按下ctrl可看到直接把process group一同砍掉


405235035 資工二 王博輝 b121417393@gmail.com
