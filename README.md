# MQTT-SN Publisher

This is an MQTT-SN client in RIOT for the "avr-rss2"
board, using the `emcute` MQTT-SN implementation in RIOT.

Data is published in SenML format. MQTT-SN runs on top of UDP,
which, unlike TCP, does not support data segmentation. Therefore,
care is taken to send complete SenML records in each UDP packet, so
that each packet can be processed individually. See
**Record Format** below.

## Files

* **Makefile** RIOT-style Makefile.
* **mqttsn_publisher.c/mqttsn_publisher.h** The main part of the MQTT-SN
client, using `emcute` to publish sensor reports.
* **report.c/report.h** Report building. Collects data to report and
constructs the MQTT-SN payload. Also containts preprocessor macros to
group strings into _records_ (see below).
* **gnrc_rpl.c** Generate RPL status reports and statistics from RIOT's gnrc_rpl implementation.
* **platform.c** Generate platform-specific reports, such as boot
information and device reports.

## Record Format

Reports are organized as sequences of _records_. A record is simply a
piece of text that should be trated as a single unit, published
together in the same MQTT-SN report. The idea is that records
represent complete SenML elements. 

Since it is convenient to build MQTT-SN reports gradually through
string concatenation, we use records as away to group larger pieces of
text together.

* `RECORD_START(STR, LEN)` Start building a record in character buffer `STR` of
size `LEN`.
* `RECORD_END(NREAD)` End a record. The variable `NREAD` will be
assigned the total number of characters that were written to the record.
* `PUTFMT(ARGS)` Write printf-formatted data to the records. Must appear within a
`RECORD_START/RECORD_END` pair.
* `RECORD_ADD(N)` Advance the number of characters in a record with
`N`. Intended to be used together witrh `RECORD_STR/RECORD_LEN` for
external functions to add data to a record.
* `RECORD_STR()/RECORD_LEN()` Return the current write position and size
of a record. Must appear within a `RECORD_START/RECORD_END` pair.

If an entire record cannot be written to a character buffer, none of
it will be written, and an early return is made from the current
function.
The return value will be the original size of the
character buffer, that is, the value of LEN passed to `RECORD_START()`.

When an early return is made from a report generator function, it
means that all data could not be written to the report, and the
remaining data should be placed in the next report. Therefore, a
report generator needs to maintain state to keep track of how much data
has been generated. See examples below.

## Adding Own Reports

First write your report generator, starting from one of the examples:

* `platform.c:boot_report()` is a
simple example of a short report generator with one record.
* `mqttsn_publisher.c:mqttsn_report()` is an example of a report generator for several records.
* `gnrc_rpl.c:rpl_report()` is a more elaborate example, where
the report generator consists of several functions.

The next step is to schedule the report generator to be called. Update
`report.c:next_report_gen()` accordingly.
