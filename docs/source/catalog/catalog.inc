The :cpp:class:`Catalog_Namespace::Catalog` is the top-level container for schemas associated with an individual database. When the OmniSciDB data directory is initialized, an initial "default" database is created. OmniSciDB always has at least one catalog at startup. The ``Catalog`` maintains the following objects:

- Tables
- Columns
- Views
- Dashboards
- mapd_dictionaries

Data is persisted using a *SQLite* database per catalog in the `catalogs` directory, with the name of the *SQLite* database file corresponding to the database name. Just like the `SysCatalog`, a `Catalog` loads all data from SQLite on creation and flushes writes to SQLite immediately.

Below is a UML representation of the schema of the SQLite database backing a catalog:

.. image:: ../img/catalog/cat_schema.png
    :height: 400

****************************************
Tables
****************************************

Tables metadata for all tables in a given database is stored in the ``mapd_tables`` *SQLite* table and read by the ``Catalog`` at startup. Table metadata is stored in the :cpp:class:`TableDescriptor` object for access by other objects in the system. For a given table, the :cpp:class:`TableDescriptor` both holds table metadata (``id``, ``fragment size``, etc) and is responsible for instantiating the ``Fragmenter`` for the table. That is, the :cpp:class:`TableDescriptor` is the primary interface to locating storage for a given table.

****************************************
Columns
****************************************

Column metadata is stored in the ``mapd_columns`` table. Each column is uniquely identified by an ID pair, mapping to a unique table ID and a unique column ID within that table. ``ColumnDescriptor`` objects pass column metadata (including the `SQL Type` of a column) throughout the system.

****************************************
Dictionaries
****************************************

Columns of type `dictionary encoded string` require a separate string dictionary. The string dictionary metadata is stored in the ``mapd_dictionaries`` SQLite table. :cpp:class:`DictDescriptor` objects encapsulate string dictionary information. Like the :cpp:class:`TableDescriptor`, :cpp:class:`DictDescriptor` is the primary interface for accessing physical string dictionary storage. 

****************************************
Sharded Tables
****************************************

OmniSciDB supports `sharded tables` -- that is, a table partitioned by a predefined column (called the `shard key`) into a pre-determined number of `physical tables`, more commonly referred to as `shards`. Sharding is used for increasing performance (by grouping like data on like devices) and for support complex queries in distributed mode (Enterprise Edition only). Internally, a sharded table is represented as a top-level `logical` table and several underlying `physical` tables (with the number of physical tables corresponding to the total shard count). This mapping is stored in the ``mapd_logical_to_physical`` *SQLite* table. 

****************************************
Views
****************************************

Views in OmniSciDB are currently not materialized during creation. Instead, the definition for the view is stored in the ``mapd_views`` *SQLite* table, and the view is materialized lazily when the view is queried. By using lazy materialization, OmniSciDB fuse the query on the view with the underlying table(s) backing the view, reducing the number of intermediate projections required and/or the amount of data that must be loaded and processed. Views are represented by a :cpp:class:`TableDescriptor` object with the `isView` boolean member set to `true`.

****************************************
Temporary Tables
****************************************

OmniSciDB supports temporary tables. Temporary tables can be created using ``CREATE TEMPORARY TABLE``. All types (except geospatial types) are supported in temporary tables. Temporary tables store their data in CPU memory and persist until the server is restarted. 

Temporary tables are identified by checking the ``persistenceLevel`` property of the :cpp:class:`TableDescriptor`. A free function, ``table_is_temporary``, is available for convenience.

Note that :ref:`calcite_parser` relies on reading the sqlite catalog files directly for unit testing. To test temporary tables, a separate JSON file is created in the catalog directory for each database containing temporary table metadata. See :ref:`calcite_omniscidb_comms` for more details.

****************************************
Dashboards
****************************************

OmniSciDB holds the dashboard state for OmniSci Immerse (Enterprise Edition only), our web-based data exploration tool. Dashboards are serialized and stored in the ``mapd_dashboards`` table. Because dashboards are built over a database and typically contain many database objects, they are considered a top-level object in the OmniSciDB catalog and object permissions model.
