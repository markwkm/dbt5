MODULES = broker_volume customer_position market_watch \
		security_detail trade_lookup trade_order trade_result trade_status \
		trade_update data_maintenance
DATA_built = broker_volume.sql customer_position.sql \
		market_watch.sql security_detail.sql trade_lookup.sql trade_order.sql \
		trade_result.sql trade_status.sql trade_update.sql \
		data_maintenance.sql

PG_CPPFLAGS=-g
PGXS := $(shell pg_config --pgxs)
include $(PGXS)
