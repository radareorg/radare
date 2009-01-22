/* radare - LGPL - Copyright 2008-2009 pancake<nopcode.org> */

#include "r_vm.h"
#include "list.h"

int r_vm_mmu_cache_write(struct r_vm_t *vm, u64 addr, u8 *buf, int len)
{
	struct r_vm_change_t *ch = (struct range_t *)malloc(sizeof(struct range_t));
	ch->from = addr;
	ch->to = addr + len;
	ch->data = (u8*)malloc(len);
	memcpy(ch->data, buf, len);
	list_add_tail(&(ch->list), &vm->mmu_cache);
	return 0;
}

int r_vm_mmu_cache_read(struct r_vm_t *vm, u64 addr, u8 *buf, int len)
{
	struct r_vm_change_t *c;
	struct list_head *pos;

	// TODO: support for unaligned and partial accesses
	list_for_each(pos, &vm->mmu_cache) {
		c = list_entry(pos, struct r_vm_change_t, list);
		if (addr >= c->from && addr+len <= c->to) {
			memcpy(buf, c->data, len);
			return 1;
		}
	}
	return 0;
}

int r_vm_mmu_read(u64 off, u8 *data, int len)
{
	if (!realio && vm_mmu_cache_read(off, data, len))
		return len;
	return radare_read_at(off, data, len);
}

int r_vm_mmu_write(u64 off, u8 *data, int len)
{
	if (!realio)
		return vm_mmu_cache_write(off, data, len);
	printf("WIte¡\n");
	return radare_write_at(off, data, len);
}

int r_vm_mmu_real(struct r_vm_t *vm, int set)
{
	return vm->realio = set;
}
