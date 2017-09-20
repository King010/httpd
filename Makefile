
bin=httpd
cc=gcc
obj=http.o main.o

LDFLAGS=-lpthread #-static
FLAGS=-D_DEBUG_
CGI_PATH=sql wwwroot/cgi-bin

.PHONY:all
all:${bin} cgi

${bin}:${obj}
	@${cc} -o $@ $^ ${LDFLAGS} -g 
	@echo "[linking] [$@] to [$^] .....done"
%.o:%.c
	@gcc -c -g $<  #${FLAGS}
	@echo "[completing] [$@] to [$^]......done"

#编译cgi脚本
cgi:
	@for i in `echo $(CGI_PATH)`;\
	do\
		cd $$i;\
		make;\
		cd -;\
	done

.PHONY:output
output:
	@mkdir -p output/wwwroot/cgi-bin
	@cp -rf log output
	@cp -rf conf output
	@cp -r wwwroot/index.html output/wwwroot/
	@cp wwwroot/cgi-bin/math_cgi output/wwwroot/cgi-bin
	@cp sql/insert_cgi output/wwwroot/cgi-bin
	@cp sql/select_cgi output/wwwroot/cgi-bin
	@cp -rf sql/lib output
	@cp httpd output
	@cp plugin/ctl_server.sh output
	@echo "output project ....done"

.PHONY:clean
clean:
	@rm -rf  ${bin} *.o output
	@for i in `echo $(CGI_PATH)`;\
	do\
		cd $$i;\
		make clean;\
		cd -;\
	done
	@echo "clean project ....done"
