# Lab1 - Shell

Ryan George & Rishabh Rastogi - Sir Tuff and Mr Tuff


## Testing

### Task 1 - Basic commands

![](Images/TASK1.png)

Testing  given commands

→ pwd

![](Images/pwd.png)

→ ls -R

![](Images/ls_-R.png)

→ sort txt/phrases.txt

![](Images/sort.png)

→ grep -n burn txt/phrases.txt

![](Images/grep.png)

→ cp txt/phrases.txt txt/phrases_copy.txt

![](Images/copy.png)

### Task 2 - Commands with redirection

Testing given commands

→ cal -y >> txt/calender.txt

![](Images/cal_append.png)

→ wc txt/phrases.txt. > txt/phrases_stats.txt

![](Images/wc_write.png)

→ tr a-z A-Z < txt/phrases.txt

![](Images/tr_read.png)

### Task 3 - Support for cd

![](Images/cd.png)

As we can see 

→ cd txt takes it into the txt file
→ cd .. takes it back to the previous directory
→ cd . is able to stay in the cwd and refreshes it
→ cd followed by nothing takes it to the home directory

### Task 4 - Piped commands

→ cat txt/phrases.txt | sort > txt/phrases_sorted.txt

![](Images/cat_pipe.png)

→ tr a-z A-Z < txt/phrases.txt | grep BURN | sort | wc -l

![There are 11, confirming that the function works as expected.](Images/tr_pipe.png)


→ ps aux | grep python | sort -k 3 -nr | head

![](Images/ps_pipe.png)




