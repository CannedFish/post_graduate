co = common.h
re = rs_epoll.h
es = engineserver.h

RouteEngine: rs_epoll.o engineserver.o main.o
	g++ *.o -o RouteEngine

rs_epoll.o: $(re) $(co) rs_epoll.cpp
	g++ -c rs_epoll.cpp
engineserver.o: $(co) $(re) $(es) engineserver.cpp
	g++ -c engineserver.cpp
main.o: $(co) $(re) $(es) main.cpp
	g++ -c main.cpp

.PHONY : clean
clean :
	rm *.o RouteEngine
