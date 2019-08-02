# Brokersaurus Rex: A regex capture to Broker event tool
----

On a terminal? Try this:

```pandoc -t man README.md  -s | man -l -```


# A What?
Let's say you have a source of events, one line per event, maybe different 
kinds of events mixed in. You want to turn these into Broker events so 
Zeek/Bro can do something useful. Brokersaurus does that.

# Usage
## Running
* Brokersaurus expects a single argument: ```--configfile=``` that points to 
a text file in libconfig format. See the ```examples``` directory for 
inspiration.

* Brokersaurus reads lines from stdin. It's up to you to decide how to do this.
(xinetd works really well!)

## Configuration
* Everything is definied in a libconfig-format config file.
* The following are required:
  * ```peer```, the hostname or ip address of the Broker peer.

  Example: ```peer = "127.0.0.1";```

  * ```peerport```, the port number of the Broker peer.
  
  Example: ```peerport = 9998;```

  * ```topic```, the Broker topic string that the peer is 
  expecting events on.
  
  Example: ```topic = "/topic/test";```

  * ```events```, a group of regex groups (use '{}').
    * Each regex group is in turn a group with two settings:
      * ```arglist```, a list (use '()', comma delmiters) of capture names
      to populate the Broker event with.
      
      Example: ```arglist = ("start_time", "hostname", "pid");```

      * ```regex```, a regular expression in string format.
      Follow the RE2 specification, and make sure to escape backslashes
      with backslashes.
      
      Example: ```regex = "sshd\\[(?P<pid>]\\d+)\\]:";```

      * The regex group's name will become the Broker event name.

* Full example:

```peer = "127.0.0.1";
peerport = 9998;
topic = "/topic/test";

events =
{
    ssh_success = 
    {
	arglist = ("start_time", "hostname", "pid", "authmethod", "username", "remaddr", "remport");
	regex = " sshd\\[(?P<pid>\\d+)\\]: Accepted (?P<authmethod>\\S+) for (?P<username>\\S+) from (?P<remaddr>\\S+) port (?P<remport>\\d+)";
    };

    ssh_fingerprint = 
    {
	arglist = ("start_time", "hostname", "pid", "keytype", "fingerprint");
	regex = " sshd\\[(?P<pid>\\d+)\\]: Found matching (?P<keytype>\\S+) key: (?P<fingerprint>\\S+)";
    };
};
```

  * This example config instructs Brokersaurus to connect to a Broker peer at
    127.0.0.1:9998 and send events to the "/topic/test" topic.

  * There are two events defined.

    * ```ssh_success``` which will create a Zeek/Bro event called 
    "ssh_success" with the arguments in order:
    ```start_time, hostname, pid, authmethod, username, remaddr, remport```
    (more on ssh_success and hostname later)

    * ```ssh_fingerprint``` which will create a Zeek/Bro event called
    "ssh_fingerprint" with the arguments in order:
    ```start_time, hostname, pid, keytype, fingerprint```
    (more on ssh_success and hostname later)

  * Example input:
  ```1564436255 ids-test sshd[21492]: Accepted publickey for root from 10.0.11.1 port 34736 ssh2: ECDSA SHA256:rpvMr7+cbvn3EN9U+7B6RaNCmciV9IStjXuDbZZ5G4I```

* Auto capture groups
  Brokersaurus will automatically parse out additional capture groups when
  a regex is matched if the line is formatted correctly.
  * Modified syslog format:
  ```<start_time = unix epoch time> <hostname = hostname>```
  * Modified syslog forwarded format:
  ```<start_time = unix epoch time> <relayhost = hostname> Message<optional> forwarded from <hostname = forwardedfor>```





