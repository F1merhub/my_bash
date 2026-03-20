# my_bash

Простой shell с поддержкой базовых команд, пайпов и редиректов.

## Возможности
- запуск обычных команд  
- пайпы: `cmd1 | cmd2`  
- ввод из файла: `cmd < file`  
- вывод в файл: `cmd > file`  
- выход по `q`  
## Примеры Использования
```bash
ls  
ls | wc  -l  
cat < input.txt  
echo hello > output.txt  
cat < input.txt | grep test > output.txt
```



После запуска появится приглашение:

```bash
>
