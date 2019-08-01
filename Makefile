objects = main.o rexitem.o worker.o pool.o 
headers = rexitem.h worker.h pool.h brokersaurusrex.h 
LDFLAGS = -lbroker -lcaf_core -lre2 -lpthread -lconfig++
#CPPFLAGS = -Og -ggdb
CPPFLAGS = -O2

brokersaurusrex : $(objects)
	$(CXX) $(LDFLAGS) -o brokersaurusrex $(objects)

$(objects) : $(headers)

clean :
	rm -f brokersaurusrex $(objects)
