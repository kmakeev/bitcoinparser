Requirements:
libQJsonRPC

Step 1: parse the blockchain into the PostgreSQL database using a multi-threaded launch postgredb -p <start> <end> 
 
Step 2: create index of the ‘btc_bitcoinaddress’ table ‘address’ field. Use the pgAdmin utility or command CREATE INDEX "btc_bitcoin_address_eb00de_idx" ON "btc_bitcoinaddress" ("address");
	Wait for index creation

Step 3: Remove duplicate addresses using postgredb -r
	In parallel spent Outpoint transaction inputs using postgredb -s
WAIT!

ATTENTION!
After matching the outputs, the base parsing needs to be maintained only for consecutive blocks using postgredb -s <hash next block in DB>