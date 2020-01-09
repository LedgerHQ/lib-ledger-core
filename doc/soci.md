# SOCI
## What's SOCI ?
The core library is using [SOCI](http://soci.sourceforge.net/index.html) as its SQL driver abstraction layer. 
It provides an easy to make SQL queries inside C++ code and the ability to plug the library to multiple types of SQL 
backends.

## Custom database engine
The core library offers a way to implement its own "database driver" by implementing a interface of host side.
The interface is heavily based on JDBC and requires to implement 9 interfaces:
- DatabaseEngine
- DatabaseConnectionPool
- DatabaseConnection
- DatabaseStatement
- DatabaseResultSet
- DatabaseResultRow
- DatabaseColumn
- DatabaseBlob
- DatabaseError

Those interfaces allows users to keep the control hover there implementations and are then reused by a proxy driver to SOCI.

## Limitations of libcore usage of SOCI

libcore uses a limited set of features offered by SOCI. The main limitation is due to the fact that in order to ease the 
code, we avoid to use exclusive database features on common functions. If you need exclusive feature like `PRAGMA` please
use it into specialized interfaces (like SQLite3Backend for example). `soci::row_id` is also out of scope because the
library should never rely on database generated values, row identifiers are different for most RDBMS. It can also be 
dangerous to use non-deterministic values.

libcore is also **not** compatible with (http://soci.sourceforge.net/doc/3.2/exchange.html#static_bulk)[SOCI bulk operations]
due to a current limitation around the custom database engine. This limitation is not per design and support could be 
added in the future. 
