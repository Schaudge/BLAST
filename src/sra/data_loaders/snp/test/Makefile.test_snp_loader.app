# $Id: Makefile.test_snp_loader.app 625148 2021-02-09 14:54:06Z ucko $


APP = test_snp_loader
SRC = test_snp_loader

REQUIRES = Boost.Test.Included $(GRPC_OPT)

CPPFLAGS = $(ORIG_CPPFLAGS) $(BOOST_INCLUDE)

LIB = ncbi_xloader_snp $(SRAREAD_LIBS) dbsnp_ptis grpc_integration test_boost $(OBJMGR_LIBS) $(CMPRS_LIB)

LIBS = $(GENBANK_THIRD_PARTY_LIBS) $(SRA_SDK_SYSLIBS) $(GRPC_LIBS) \
       $(CMPRS_LIBS) $(NETWORK_LIBS) $(ORIG_LIBS)

POST_LINK = $(VDB_POST_LINK)

CHECK_COPY = test_snp_loader.ini
CHECK_CMD = test_snp_loader
CHECK_REQUIRES = in-house-resources

WATCHERS = vasilche
