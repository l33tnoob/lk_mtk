#include <string.h>
#include <arch/ops.h>
#include <dev/mrdump.h>
#include <debug.h>
#include "aee.h"
#include "kdump.h"

#ifdef CFG_DTB_EARLY_LOADER_SUPPORT
static unsigned int mrdump_cb_addr(void)
{
    int lenp = 0;
    struct fdt_property *prop = NULL;
    int offset = fdt_node_offset_by_compatible(g_fdt, 0, "mrdump-reserved-memory");
    if(offset == 0) {
        dprintf(CRITICAL,"%s:%d invalide offset=%d\n",__func__,__LINE__,offset);
        return 0;
    }
    else{
        prop = fdt_get_property(g_fdt, offset, "reg", &lenp);
        if(!prop){
            dprintf(CRITICAL,"%s:%d get reg prperty failed\n",__func__,__LINE__);
            return 0;
        }
    }
    struct mrdump_reserve_args *mrdump_rsv_args = (struct mrdump_reserve_args *)&prop->data;

    dprintf(CRITICAL,"mrdump_rsv_args->lo_addr = %x\n",fdt32_to_cpu(mrdump_rsv_args->lo_addr));
    return fdt32_to_cpu(mrdump_rsv_args->lo_addr);
}
#else

static unsigned int mrdump_cb_addr(void)
{
    return MRDUMP_CB_ADDR;
}

#endif

static struct mrdump_control_block *mrdump_cb = NULL;

static void mrdump_query_bootinfo(void)
{
    if (mrdump_cb == NULL) {
        struct mrdump_control_block *bufp = (struct mrdump_control_block *)mrdump_cb_addr();
        if (bufp == NULL) {
            voprintf_debug("mrdump_cb is invalid 0x%x\n", (unsigned int *)mrdump_cb);
            return;
        }
        if (memcmp(bufp->sig, MRDUMP_GO_DUMP, 8) == 0) {
            bufp->sig[0] = 'X';
        } else {
            memset(bufp, 0, sizeof(struct mrdump_control_block));
            memcpy(bufp->sig, MRDUMP_VERSION, 8);
        }
        mrdump_cb = bufp;
        aee_mrdump_flush_cblock(mrdump_cb);
        voprintf_debug("Boot record found at %p[%02x%02x], cb %p\n", bufp, bufp->sig[0], bufp->sig[1], mrdump_cb);
    }
}

struct mrdump_control_block *aee_mrdump_get_params(void)
{
    mrdump_query_bootinfo();
    return mrdump_cb;
}

void aee_mrdump_flush_cblock(struct mrdump_control_block *bufp)
{
    if (bufp != NULL) {
        arch_clean_cache_range((addr_t)bufp, sizeof(struct mrdump_control_block));
    }
}
