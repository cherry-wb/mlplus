BOOSTPATH="/usr/local/include/apsara"
CXXFLAGS += -g -Wall -Wextra 
tosvm: tosvm.cpp pattern_scan.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -I$(BOOSTPATH) -lm -lpthread -lgflags -o $@ 
clean :
	rm tosvm 

