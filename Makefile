objects = main.o rexitem.o worker.o pool.o 
headers = rexitem.h worker.h pool.h brokersaurusrex.h 
LDFLAGS = -lbroker -lcaf_core -lre2 -lpthread -lconfig++ -L/opt/broker/lib
#CPPFLAGS = -O0 -ggdb
CPPFLAGS = -O2 -I/opt/broker/include --std=gnu++17

brokersaurusrex : $(objects)
	$(CXX) $(LDFLAGS) -o brokersaurusrex $(objects)

$(objects) : $(headers)

clean :
	rm -f brokersaurusrex $(objects)
