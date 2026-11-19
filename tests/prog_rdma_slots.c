/*
 * nfs_set_rdma_slots() and the rdma-slots URL argument, no server needed.
 *
 * Without RDMA support in the library every call has to fail, with it the
 * value has to be within RPC_RDMA_MIN_SLOTS..RPC_RDMA_MAX_SLOTS.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nfsc/libnfs.h"

static int failures;

static void
expect(int cond, const char *what)
{
	if (!cond) {
		fprintf(stderr, "FAIL: %s\n", what);
		failures++;
	}
}

static int
url_ok(const char *url)
{
	struct nfs_context *nfs = nfs_init_context();
	struct nfs_url *u;
	int ok;

	if (nfs == NULL) {
		fprintf(stderr, "failed to init context\n");
		exit(1);
	}
	u = nfs_parse_url_full(nfs, url);
	ok = u != NULL;
	if (u) {
		nfs_destroy_url(u);
	}
	nfs_destroy_context(nfs);
	return ok;
}

int
main(void)
{
	struct nfs_context *nfs = nfs_init_context();
	int rdma;
	char url[128];
	int n;

	if (nfs == NULL) {
		fprintf(stderr, "failed to init context\n");
		return 1;
	}
	/* Does this build of libnfs have RDMA? */
	rdma = nfs_set_transport(nfs, RPC_TRANSPORT_RDMA) == 0;

	expect((nfs_set_rdma_slots(nfs, RPC_RDMA_MIN_SLOTS) == 0) == rdma,
	       "minimum slots");
	expect((nfs_set_rdma_slots(nfs, RPC_RDMA_DEFAULT_SLOTS) == 0) == rdma,
	       "default slots");
	expect((nfs_set_rdma_slots(nfs, RPC_RDMA_MAX_SLOTS) == 0) == rdma,
	       "maximum slots");
	expect(nfs_set_rdma_slots(nfs, RPC_RDMA_MIN_SLOTS - 1) < 0, "0 slots");
	expect(nfs_set_rdma_slots(nfs, -1) < 0, "negative slots");
	expect(nfs_set_rdma_slots(nfs, RPC_RDMA_MAX_SLOTS + 1) < 0,
	       "too many slots");
	nfs_destroy_context(nfs);

	n = snprintf(url, sizeof(url), "nfs://127.0.0.1/dir/file?rdma-slots=%d",
		     RPC_RDMA_DEFAULT_SLOTS * 2);
	expect(n > 0 && url_ok(url) == rdma, "url with a valid rdma-slots");
	expect(url_ok("nfs://127.0.0.1/dir/file?proto=rdma&rdma-slots=1") == rdma,
	       "url with proto=rdma and rdma-slots");
	expect(!url_ok("nfs://127.0.0.1/dir/file?rdma-slots=0"), "url with 0");
	expect(!url_ok("nfs://127.0.0.1/dir/file?rdma-slots=257"),
	       "url with too many");
	expect(!url_ok("nfs://127.0.0.1/dir/file?rdma-slots=-4"),
	       "url with a negative value");
	expect(!url_ok("nfs://127.0.0.1/dir/file?rdma-slots=abc"),
	       "url with a non-number");
	expect(!url_ok("nfs://127.0.0.1/dir/file?rdma-slots=8x"),
	       "url with trailing garbage");
	expect(!url_ok("nfs://127.0.0.1/dir/file?rdma-slots="),
	       "url with an empty value");

	if (failures) {
		return 1;
	}
	printf("rdma-slots: OK (%s)\n", rdma ? "RDMA build" : "no RDMA");
	return 0;
}
