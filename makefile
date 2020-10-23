

CC=gcc # 使用gcc编译器
WARN_PRT=-Wall # 输出所有警告
# 需要链接的库
RT=-lrt
POSIXT=-lpthread
MATH=-lm 

prod: # 单独编译文件prod.c
	$(CC) $(WARN_PRT) prod.c -o prod.out $(RT) $(POSIXT) $(MATH)

cons: # 单独编译文件cons.c
	$(CC) $(WARN_PRT) cons.c -o cons.out $(RT) $(POSIXT) $(MATH)

dph: # 单独编译文件dph.c
	$(CC) $(WARN_PRT) dph.c -o dph.out $(RT) $(POSIXT)
    
# sleep: # 单独编译文件sleep.c
# 	$(CC) $(WARN_PRT) sleep.c -o sleep.out

all: # 编译所有.c文件
	$(CC) $(WARN_PRT) prod.c -o prod.out $(RT) $(POSIXT) $(MATH)
	$(CC) $(WARN_PRT) cons.c -o cons.out $(RT) $(POSIXT) $(MATH)
	$(CC) $(WARN_PRT) dph.c -o dph.out $(RT) $(POSIXT)
	# $(CC) $(WARN_PRT) sleep.c -o sleep.out
    
clean: # 删除所有.out文件
	rm -rf prod.out
	rm -rf cons.out
	rm -rf dph.out
