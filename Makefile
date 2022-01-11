CXX=g++
CXXFLAGS=-std=c++17
CXXFLAGS+=-ggdb
CXXFLAGS+=-o3
CXXFLAGS+=-I /home/ubuntu/opentelemetry-cpp/install/include
LDFLAGS=-L/home/ubuntu/opentelemetry-cpp/install/lib
LDFLAGS+=-lopentelemetry_resources
LDFLAGS+=-lopentelemetry_trace
LDFLAGS+=-lopentelemetry_common
#LDFLAGS+=-lopentelemetry_exporter_ostream_span
LDFLAGS+=-lopentelemetry_exporter_zipkin_trace
LDFLAGS+=-lhttp_client_curl
LDFLAGS+=-lpthread -lcurl
TARGET=test

$(TARGET): Makefile test.cpp
	$(CXX) $(CXXFLAGS) -o $@ test.cpp $(LDFLAGS)

clean:
	rm -f $(TARGET)
