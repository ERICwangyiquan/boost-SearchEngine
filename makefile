PARSER=parser
DBG=debug
HTTP_SERVER=http_server
cc=g++

.PHONY:all
all: $(PARSER) $(HTTP_SERVER)

$(PARSER):parser.cc
	$(cc) -o $@ $^ -lboost_system -lboost_filesystem -std=c++11
# $(DBG):debug.cc
# 	$(cc) -o $@ $^ -ljsoncpp -std=c++11
$(HTTP_SERVER):http_server.cc
	$(cc) -o $@ $^  -ljsoncpp -lpthread -std=c++11

 .PHONY:clean
 clean:
	rm -f $(PARSER) $(DBG) $(HTTP_SERVER)


# You should input this: 
# 			./parser
# 			nohup ./http_server > log/log.txt 2>&1 &