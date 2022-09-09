# tp-size

Study of DUNE FD DAQ trigger primitive size and compression

If we want to store all of the DUNE FD trigger primitives (TPs), they have to fit inside the 30 PB/year data limit. The dominant source of TPs is likely to be Argon39 decays, which occur at 10 MHz per 10kton module. Assuming every single Ar39 decay produces a trigger primitive in each of the three views, that gives a maximum size of TPs of:

```
>>> PB=1024*1024*1024*1024*1024
>>> seconds_per_year=60*60*24*365
>>> n_views=3
>>> Ar39rate=10e6
>>> 30*PB/(n_views*Ar39rate*seconds_per_year)
35.7 bytes
```

The TP format described in [this talk](https://indico.fnal.gov/event/47053/contributions/207915/attachments/139684/175399/DS_data_products.pdf) (hereafter `TP1`) is 46 bytes in size, with fields:

```cpp
   uint64_t time_start;
   uint64_t time_peak;
   uint32_t time_over_threshold;
   uint32_t channel;
   uint32_t adc_integral;
   uint16_t adc_peak;
   uint32_t detid;
   uint32_t type;
   uint16_t algorithm;
   uint16_t version;
   uint32_t flag;
```

The uncompressed size of the TP exceeds the 35 bytes/TP available, but we can compress them. This repo contains some code to estimate a reasonable compression factor based on TP data from ProtoDUNE-SP. This data was taken with [ptmp](https://github.com/brettviren/ptmp), whose TP format does not have fields corresponding to `time_peak`, `adc_peak`, `type`, `algorithm`, `version` or `flag`. For this study, I have converted the ptmp TPs into `TP1` format by setting `time_peak` to `time_over_threshold/2` and `adc_peak` to `adc_sum/10`. `type`, `algorithm`, `version` and `flag` are each set to constants.

The `TP1` format TPs are saved into a ROOT TTree. The TTree on-disk format is columnar, meaning that all the entries of a single field are stored together. In our case, this is favourable for compression, and makes it possible to make meaningful statements about the space taken up by each field. With the data recorded from ProtoDUNE-SP and converted to `TP1` format, we get the following compression factors, with ROOT's default compression algorithm and compression level:

```
   Uncompressed  |   Compressed    |  Compression | Field name
 Bytes per entry | Bytes per entry |  Factor      |
============================================================================
            8    |         2.79    |       2.9    | time_start
            8    |         1.38    |       5.8    | time_peak
            4    |         1.24    |       3.2    | time_over_threshold
            4    |         1.23    |       3.2    | channel
            4    |         2.31    |       1.7    | adc_integral
            2    |         1.47    |       1.4    | adc_peak
            4    |         0.03    |     147.6    | detid
            4    |         0.03    |     148.4    | type
            2    |         0.01    |     158.6    | algorithm
            2    |         0.01    |     159.6    | version
            4    |         0.02    |     161.5    | flag
----------------------------------------------------------------------------
           46    |        10.53    |       4.4    | Overall
```

The bottom-line number here is the overall compressed bytes per entry: 10.5, which fits within the 35 bytes available.

We can gain a little bit by sorting the TPs by `time_start` and storing the _difference_ between `time_start` and the previous entry's `time_start`:

```
   Uncompressed  |   Compressed    |  Compression | Field name
 Bytes per entry | Bytes per entry |  Factor      |
==============================================================================
            8    |         1.32    |       6.1    | time_start
            8    |         1.38    |       5.8    | time_peak
            4    |         1.24    |       3.2    | time_over_threshold
            4    |         1.20    |       3.3    | channel
            4    |         2.31    |       1.7    | adc_integral
            2    |         1.47    |       1.4    | adc_peak
            4    |         0.03    |     141.1    | detid
            4    |         0.03    |     141.9    | type
            2    |         0.01    |     151.2    | algorithm
            2    |         0.01    |     152.2    | version
            4    |         0.03    |     154.1    | flag
------------------------------------------------------------------------------
           46    |         9.03    |       5.1    | Overall
```

Done naively, this means we have to read all the entries up to the current entry in order to find its absolute `time_start`. A more realistic approach would store groups of TPs, with the absolute timestamp of the first TP in the group stored. Then we only have to read the entries in the desired TP's group to find the absolute timestamp. With moderate-sized groups, the compression ratio would be little changed.

ROOT's file format has a "compression level" which can be increased. Doing so can reduce the per-TP size to 7.5 bytes, but at the cost of an order-of-magnitude increase in time to compress the data.

This study is done with ROOT files, but we're using HDF5 files in dunedaq. HDF5 has its own compression functionality, so I would imagine that similar compression ratios can be achieved. For an example of compressing raw WIB data with HDF5, see https://github.com/philiprodrigues/wib_hdf5_compress/ . The techniques used there should carry over to TPs, if HDF5 "compound datasets" are used to store the TPs.
