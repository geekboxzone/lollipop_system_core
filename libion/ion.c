/*
 *  ion.c
 *
 * Memory Allocator functions for ion
 *
 *   Copyright 2011 Google, Inc
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#define LOG_TAG "ion"

#include <cutils/log.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>

#include <ion/sofia_ion.h>
#include <ion/ion.h>

int ion_open()
{
        int fd = open("/dev/ion", O_RDWR);
        if (fd < 0)
                ALOGE("open /dev/ion failed!\n");
        return fd;
}

int ion_close(int fd)
{
        return close(fd);
}

static int get_ioctl_str(int req, char* req_name)
{
	switch (req) {
	case ION_IOC_ALLOC:
		sprintf(req_name, "ION_IOC_ALLOC");
		break;
	case ION_IOC_FREE:
		sprintf(req_name, "ION_IOC_FREE");
		break;
	case ION_IOC_MAP:
		sprintf(req_name, "ION_IOC_MAP");
		break;
	case ION_IOC_SHARE:
		sprintf(req_name, "ION_IOC_SHARE");
		break;
	case ION_IOC_IMPORT:
		sprintf(req_name, "ION_IOC_IMPORT");
		break;
	case ION_IOC_SYNC:
		sprintf(req_name, "ION_IOC_SYNC");
		break;
	case ION_IOC_CUSTOM:
		sprintf(req_name, "ION_IOC_CUSTOM");
		break;
	case ION_IOC_GET_PHYS:
		sprintf(req_name, "ION_IOC_GET_PHYS");
		break;
	case ION_IOC_GET_SHARE_ID:
		sprintf(req_name, "ION_IOC_GET_SHARE_ID");
		break;
	case ION_IOC_SHARE_BY_ID:
		sprintf(req_name, "ION_IOC_SHARE_BY_ID");
		break;
	default:
		sprintf(req_name, "UNKNOWN");
		break;
	}
	return 0;
}

static int ion_ioctl(int fd, int req, void *arg)
{
        int ret = ioctl(fd, req, arg);
        if (ret < 0) {
				char req_name[64];
				get_ioctl_str(req, req_name);
                ALOGE("ioctl %s(%x) failed with code %d: %s\n", req_name, req,
                       ret, strerror(errno));
                return -errno;
        }
        return ret;
}

int ion_alloc(int fd, size_t len, size_t align, unsigned int heap_mask,
	      unsigned int flags, struct ion_handle **handle)
{
        int ret;
        struct ion_allocation_data data = {
                .len = len,
                .align = align,
                .heap_mask = heap_mask,
                .flags = flags,
        };

        ret = ion_ioctl(fd, ION_IOC_ALLOC, &data);
        if (ret < 0)
                return ret;
        *handle = data.handle;
        return ret;
}

int ion_free(int fd, struct ion_handle *handle)
{
        struct ion_handle_data data = {
                .handle = handle,
        };
        return ion_ioctl(fd, ION_IOC_FREE, &data);
}

int ion_map(int fd, struct ion_handle *handle, size_t length, int prot,
            int flags, off_t offset, unsigned char **ptr, int *map_fd)
{
        struct ion_fd_data data = {
                .handle = handle,
        };

        int ret = ion_ioctl(fd, ION_IOC_MAP, &data);
        if (ret < 0)
                return ret;
        *map_fd = data.fd;
        if (*map_fd < 0) {
                ALOGE("map ioctl returned negative fd\n");
                return -EINVAL;
        }
        *ptr = mmap(NULL, length, prot, flags, *map_fd, offset);
        if (*ptr == MAP_FAILED) {
                ALOGE("mmap failed: %s\n", strerror(errno));
                return -errno;
        }
        return ret;
}

int ion_share(int fd, struct ion_handle *handle, int *share_fd)
{
        int map_fd;
        struct ion_fd_data data = {
                .handle = handle,
        };

        int ret = ion_ioctl(fd, ION_IOC_SHARE, &data);
        if (ret < 0)
                return ret;
        *share_fd = data.fd;
        if (*share_fd < 0) {
                ALOGE("share ioctl returned negative fd\n");
                return -EINVAL;
        }
        return ret;
}

int ion_alloc_fd(int fd, size_t len, size_t align, unsigned int heap_mask,
		 unsigned int flags, int *handle_fd) {
	struct ion_handle *handle;
	int ret;

	ret = ion_alloc(fd, len, align, heap_mask, flags, &handle);
	if (ret < 0)
		return ret;
	ret = ion_share(fd, handle, handle_fd);
	ion_free(fd, handle);
	return ret;
}

int ion_import(int fd, int share_fd, struct ion_handle **handle)
{
        struct ion_fd_data data = {
                .fd = share_fd,
        };

        int ret = ion_ioctl(fd, ION_IOC_IMPORT, &data);
        if (ret < 0)
                return ret;
        *handle = data.handle;
        return ret;
}

int ion_sync_fd(int fd, int handle_fd)
{
    struct ion_fd_data data = {
        .fd = handle_fd,
    };
    return ion_ioctl(fd, ION_IOC_SYNC, &data);
}

int ion_get_phys(int fd, struct ion_handle *handle, unsigned long *phys)
{
    struct ion_phys_data phys_data;
    struct ion_custom_data data;

    phys_data.handle = handle;
    phys_data.phys = 0;

    data.cmd = ION_IOC_GET_PHYS;
    data.arg = (unsigned long)&phys_data;

    int ret = ion_ioctl(fd, ION_IOC_CUSTOM, &data);
    if (ret<0)
        return ret;

    *phys = phys_data.phys;

    return 0;
}

int ion_get_share_id(int fd, int share_fd, unsigned int *id)
{
	struct ion_share_id_data share_data = {
		.fd = share_fd,
		.id = 0,
	};

	struct ion_custom_data data = {
		.cmd = ION_IOC_GET_SHARE_ID,
		.arg = (unsigned long)&share_data,
	};

	int ret = ion_ioctl(fd, ION_IOC_CUSTOM, &data);
	if (ret<0)
		return ret;

	*id = share_data.id;

	return 0;
}

int ion_share_by_id(int fd, int *share_fd, unsigned int id)
{
	struct ion_share_id_data share_data = {
		.fd = 0,
		.id = id,
	};

	struct ion_custom_data data = {
		.cmd = ION_IOC_SHARE_BY_ID,
		.arg = (unsigned long)&share_data,
	};

	int ret = ion_ioctl(fd, ION_IOC_CUSTOM, &data);
	if (ret<0)
		return ret;

	*share_fd = share_data.fd;

	return 0;
}

enum {
	CACHE_CLEAN = 0,
	CACHE_INV,
	CACHE_CLEAN_INV,
};

static int clean_inv_cache(int fd, struct ion_handle *handle, void *base, size_t size, int offset, int ops)
{
	struct ion_flush_data flush_data = {
		.handle = handle,
		.vaddr = base,
		.offset = offset,
		.length = size,
	};

	struct ion_custom_data data = {
		.arg = (unsigned long)&flush_data,
	};

	switch(ops) {
	case CACHE_CLEAN:
		data.cmd = ION_IOC_CLEAN_CACHES;
		break;
	case CACHE_INV:
		data.cmd = ION_IOC_INV_CACHES;
		break;
	case CACHE_CLEAN_INV:
		data.cmd = ION_IOC_CLEAN_INV_CACHES;
		break;
	}

	int ret = ion_ioctl(fd, ION_IOC_CUSTOM, &data);

	if (ret<0)
		return ret;

    return 0;
}

inline int ion_clean_cache(int fd, struct ion_handle *handle, void *base, size_t size, int offset)
{
	return clean_inv_cache(fd, handle, base, size, offset, CACHE_CLEAN);
}

inline int ion_inv_cache(int fd, struct ion_handle *handle, void *base, size_t size, int offset)
{
	return clean_inv_cache(fd, handle, base, size, offset, CACHE_INV);
}

inline int ion_clean_inv_cache(int fd, struct ion_handle *handle, void *base, size_t size, int offset)
{
	return clean_inv_cache(fd, handle, base, size, offset, CACHE_CLEAN_INV);
}

