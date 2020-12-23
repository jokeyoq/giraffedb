objs = main.o container.o server.o \
		map.o strlist.o user.o cfgutils.o \
		netutils.o strutils.o
VPATH = dast:core:security:utils:api
giraffedb: $(objs)
	gcc -o giraffedb $(objs) -lpthread
container.o: container.h
server.o: server.h
map.o: map.h
strlist.o: strlist.h
user.o: user.h
cfgutils.o: cfgutils.h
netutils.o: netutils.h
strutils.o: strutils.h
".PHONY": clean
clean: 
	rm -r giraffedb $(objs)
