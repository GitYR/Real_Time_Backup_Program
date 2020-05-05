# 2019_lsp_project3

한글 설명은 PDF 첨부하였습니다 -> 과제 명세서
This program was written at(?) Ubuntu Linux.

[CAUTION]

I'm not a native english speaker, so please consider it when you read this script.

[OVERVIEW]

The project is a real-time backup program that can add a file or directory user want to backup, can delete them and can restore them by backup files or directories.

If you execute this program, you have to input a path to build as backup directory.
  - If there is no argument(path), the program will build a backup directory in the current working directory.
  - If the number of arguments are over two, the program will print 'usage' and exit itself.
  - If the entered-argument can't be found, the program will print 'usage' and exit itself.
  - If the entered-argument is not a directory file or its accessing-authority is invalid, the program will print 'usage' and     exit itself. 
  
When the program is executed, it will print a prompt such as '[MY_STUDENT_ID] >'. 
There are valid commands such as 'add','remove','compare','recover','list','ls','vi(m)',and 'exit'. 
If you enter an invalid command, the program will handle it and be returned to prompt.
Until the 'exit' command will be entered, the program will be running.

[COMMAND]

1. 'add'
  
  use example : "add <FILENAME> [PERIOD] [OPTION]"
  
  <FILENAME> : It is necessary to enter <FILENAME> when you command 'add'.
  
    If you want to add a new backup file, enter the 'add' command. Then, the program will add <FILENAME> in a backup list.
    Backup list is a linked list structure. you can use absolute path and relative path both in <FILENAME>.
    Only 'regular' type file can be used in <FILENAME> and if the entered-file already exists in backup list, the program will
    go back to prompt.
    
  [PERIOD] : It is also necessary to enter [PERIOD] when you command 'add'.
  
    As the title says, it is a period of time interval the file has to be backup. The type of [PERIOD] is 'Integer', not    
    'Float'. 
    
  [OPTION] : It is not essential to enter [OPTION] when you command 'add',but is useful to enter [OPTION].
  
    '-m' : In every entered period, Only when the file's mtime is modified, the program will be executed.
    
    '-n [NUMBER]' : [NUMBER] is a maximum number of backup files. If the number of backup files is over [NUMBER], the oldest 
                    file will be deleted. [NUMBER] is an integer,not a float type and when you want to use '-n' option, you
                    have to enter [NUMBER]. The range of [NUMBER] is from 1 to 100.
    
    '-t [TIME]' : '-t' option is to set up the storage period of backup files by using [TIME]. [TIME] is an integer type and 
                  it means seconds. In every period the program makes a backup file and check the creation time of every      
                  backup file. If the creation time is over [TIME], that file will be deleted. The range of [TIME] is from 60 
                  to 1200.
    
    '-d' : If you enter a directory, the program will add every file in that directory to backup list. The maximum number of
           directory you can add is only one and if you enter it, the program will add every files, including its sub 
           directory.
    
2. 'remove'

  use example : "remove <FILENAME> [OPTION]
  
  <FILENAME> : It is necessary to enter <FILENAME> when you command 'remove'.
  
    If you don't want to make any more backup files, you can use this command to remove <FILENAME> from backup list.
    The detailed explanation is same with <FILENAME> in 'add'.
    
  [OPTION]
  
    -a' : All files will be deleted from backup list. If you use '-a' option, <FILENAME> is not necessary.

3. 'compare'

  use example : "compare <FILENAME1> <FILENAME2>
  
  When you use this command, the program will compare <FILENAME1> and <FILENAME2> by 'mtime' and file sizes.
  If 'mtime' and file sizes are same, the program will treat them as same files. The detailed explanation of <FILENAME> is 
  same with <FILENAME> in 'add'.

4. 'recover'

  use example : "recover <FILENAME> [OPTION]
  
  When you use this command, the program will change the original file into backup file. If the backup file exists,
  the program will print the list of backup times and those sizes in ascending order by backup time.
  You can choose a file you want to change. When you choose the file then the program will print the contents of changed file.
  
  <FILENAME> : this is same with <FILENAME> in 'add'.
  
  [OPTION]
  
    '-n <NEWFILE>' : If you use this option, the program will create a <NEWFILE>,not changing the original file into backup
                     file. If <NEWFILE> already exists, the program will handle the error and be returned to prompt.

5. 'list' : When you use this, the program will print all backup file list.

6. 'ls' and 'vi' : these commands is same with 'ls' and 'vi' in Linux Terminal.

7. 'exit' : exit the program

  
