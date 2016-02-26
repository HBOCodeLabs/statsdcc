## statsdcc
- - -
Statsdcc is a [Statsd](https://github.com/etsy/statsd#statsd-)-compatible high performance multi-threaded network daemon written in C++. It aggregates stats and sends aggregates to backends.


## Dependencies
- - -
* [openssl-devel-1.0.1e](https://www.openssl.org/source/) - libcrypto for MD4
* [boost-devel-1.58.0](http://www.boost.org/) - lockfree queue
* [boost-regex](http://www.boost.org/doc/libs/1_59_0/libs/regex/doc/html/index.html) - regex
* [gperftools-devel-2.0](http://goog-perftools.sourceforge.net/doc/tcmalloc.html) - tcmalloc
* [libmicrohttpd-0.9.33](http://www.gnu.org/software/libmicrohttpd/) - http server
* [jsoncpp-1.6.1](https://github.com/open-source-parsers/jsoncpp/tree/1.6.1) - json parser
* [googletest-1.7.0](https://code.google.com/p/googletest/) - unit tests

## Compilation
- - -
```bash
git clone https://github.com/wayfair/statsdcc.git
cd statsdcc
git submodule init
git submodule update
mkdir build && cd build
cmake -D test=true ..
make
ctest
```

## Usage
- - -
To run proxy
```bash
./src/proxy <config file>
```
To run aggregator
```bash
./src/statsdcc <config file>
```

## Configuration
- - -
Example configuration files are in [statsdcc/etc](Logger)

### Commmon Configuration Variables

- 	`servers`: A list of server threads 

	- 	`udp`: This variable is optional if tcp server is configured.
			   To configure more than one threads to listen for metrics set variable threads. 
			   To override the size of udp recv buffer set recv_buffer variable. 
			   If recv_buffer not specified defaults to 8388608 bytes.
			   
		```
		"udp": {
			"port": 9090,
			"threads": 3,
			"recv_buffer": 33554431
		}
		```
		
	-	`tcp`: This variable is optional if udp server is configured.
	
		```			
		"tcp": {
			"port": 9090
		}
		```
		
	-	`http`: Http server listens for status check queries, by default uses port 8080.
	
		```
		"http": {
			"port": 8000
		}
		```
			
-	`workers`: Number of worker threads to compute aggregations. If not specified defaults to 1.
	
-	`log_level`: Possible values "debug"/"info"/"warn"/"error". If not specified defaults to "warn". 
				 If set only logs with the specified level or higher will be logged.
	
### Aggregator Configuration Variables

- 	`name`: A custom name to the program. This name will be prefixed to status metrics generated by statsdcc. The default value is "statsdcc". 

-	`frequency`: Interval in seconds to flush metrics to each backend. If not specified defaults to 10.
	
-	`percentiles`: For time metrics, calculate the Nth percentile(s). If not specified defaults to [90].

	```
	"percentiles": [80, 90]
	```
	
-	`backends`:	List of backends to flush aggregations to. At least one of the following backends should be set. 
	
	-	`stdout`: Set to true to dump aggregations to stdout.
		
	-	`carbon`: List of backend carbon instances.
	
		```
		"carbon": [
			{
				"shard": "1",
				"host": "localhost",
				"port": 3101,
				"vnodes": 1000,
				"weight": 1
			},
			{
				"shard": "2",
				"host": "localhost",
				"port": 3102,
				"vnodes": 1000,
				"weight": 1
			}
		]
		```
		
		-	shard: Key for this node used while building HashRing for consistent hasing.
				
		-	host: Hostname for carbon instance. If not specified defaults to "127.0.0.1".
				
		-	port: Tcp port number on which the carbon instance is listeningfor metrics. If not specified defaults to 3000.
				
		-	vnodes: The amount of virtual nodes per server. 
					Used for consistent hasing. 
					Larger number gives bigger distribution in the HashRing. 
					If not specified defaults to 1000.
					
		-	weight: Similar to vnodes larger weight gives bigger distribution in the HashRing. 
					If not specified defaults to 1.

-	`repeaters`: Using repeaters you can flush the aggregates to another statsdcc-aggregator or statsdcc-proxy.

	```
	"repeaters": [
		{
			"host": "localhost",
			"port": 8126
		}
	]
	```

### Proxy Configuration Variables

-	`backends`:	List of backends to send metrics to, atleast one of the following backends should be set. 
	
	-	`stdout`: Set to true to dump aggregations to stdout.
	
	-	`aggregator`: List of backend aggregator instances.
	
	```
	"aggregator": [
		{
			"host": "localhost",
			"port": 9090
		},
		{
			"host": "localhost",
			"port": 9091
		}
	]
	```

## Developer's References
- - -
* UNIX Network Programming: W. Richard Stevens
* Advanced Programming in the UNIX Environment: W. Richard Stevens, Stephen A. Rago
* C++ Concurrency in Action: Practical Multithreading: Anthony Williams
* Effective Modern C++: Scott Meyers