/*
 * Copyright (C) 2019 Assured Information Security, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "driver.h"
#include <common.h>
#include <microv/builderinterface.h>

#include <bfdebug.h>
#include <bftypes.h>
#include <bfconstants.h>
#include <bfplatform.h>

/* -------------------------------------------------------------------------- */
/* Helper Functions                                                            */
/* -------------------------------------------------------------------------- */

int64_t
copy_from_user(void *dst, const void*src, uint64_t num)
{
    PMDL mdl = NULL;
    PVOID buffer = NULL;

    try {
        ProbeForRead((void *)src, num, sizeof(UCHAR));
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
        BFALERT("ProbeForRead failed\n");
        return -1;
    }

    mdl = IoAllocateMdl((void *)src, (ULONG)num, FALSE, TRUE, NULL);
    if (!mdl) {
        BFALERT("IoAllocateMdl failed\n");
        return -1;
    }

    try {
        MmProbeAndLockPages(mdl, UserMode, IoReadAccess);
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
        BFALERT("MmProbeAndLockPages failed\n");
        IoFreeMdl(mdl);
        return -1;
    }

    buffer = MmGetSystemAddressForMdlSafe(mdl, NormalPagePriority | MdlMappingNoExecute);
    if (!buffer) {
        BFALERT("MmGetSystemAddressForMdlSafe failed\n");
        MmUnlockPages(mdl);
        IoFreeMdl(mdl);
        return -1;
    }

    RtlCopyMemory(dst, buffer, num);

    MmUnlockPages(mdl);
    IoFreeMdl(mdl);

    return 0;
}

// https://github.com/Microsoft/Windows-driver-samples/blob/master/general/ioctl/wdm/sys/sioctl.c

/* -------------------------------------------------------------------------- */
/* Queue Functions                                                            */
/* -------------------------------------------------------------------------- */

static long
ioctl_create_vm(struct create_vm_args *args)
{
    int64_t ret;

    void *image = 0;
    void *initrd = 0;
    void *cmdl = 0;

    if (args->image != 0 && args->image_size != 0) {
        image = platform_alloc_rw(args->image_size);
        if (image == NULL) {
            BFALERT("IOCTL_CREATE_VM: failed to allocate memory for image\n");
            goto failed;
        }

        ret = copy_from_user(image, args->image, args->image_size);
        if (ret != 0) {
            BFALERT("IOCTL_CREATE_VM: failed to copy image from userspace\n");
            goto failed;
        }

        args->image = image;
    }

    if (args->initrd != 0 && args->initrd_size != 0) {
        initrd = platform_alloc_rw(args->initrd_size);
        if (initrd == NULL) {
            BFALERT("IOCTL_CREATE_VM: failed to allocate memory for initrd\n");
            goto failed;
        }

        ret = copy_from_user(initrd, args->initrd, args->initrd_size);
        if (ret != 0) {
            BFALERT("IOCTL_CREATE_VM: failed to copy initrd from userspace\n");
            goto failed;
        }

        args->initrd = initrd;
    }

    if (args->cmdl != 0 && args->cmdl_size != 0) {
        cmdl = platform_alloc_rw(args->cmdl_size);
        if (cmdl == NULL) {
            BFALERT("IOCTL_CREATE_VM: failed to allocate memory for cmdl\n");
            goto failed;
        }

        ret = copy_from_user(cmdl, args->cmdl, args->cmdl_size);
        if (ret != 0) {
            BFALERT("IOCTL_CREATE_VM: failed to copy cmdl from userspace\n");
            goto failed;
        }

        args->cmdl = cmdl;
    }

    ret = common_create_vm(args);
    if (ret != BF_SUCCESS) {
        BFDEBUG("common_create_vm failed: %llx\n", ret);
        goto failed;
    }

    args->image = 0;
    args->initrd = 0;
    args->cmdl = 0;

    platform_free_rw(image, args->image_size);
    platform_free_rw(initrd, args->initrd_size);
    platform_free_rw(cmdl, args->cmdl_size);

    BFDEBUG("IOCTL_CREATE_VM: succeeded\n");
    return BF_IOCTL_SUCCESS;

failed:

    args->image = 0;
    args->initrd = 0;
    args->cmdl = 0;

    platform_free_rw(image, args->image_size);
    platform_free_rw(initrd, args->initrd_size);
    platform_free_rw(cmdl, args->cmdl_size);

    BFALERT("IOCTL_CREATE_VM: failed\n");
    return BF_IOCTL_FAILURE;
}

static long
ioctl_destroy_vm(domainid_t *args)
{
    int64_t ret;
    domainid_t domainid = *args;

    ret = common_destroy_vm(domainid);
    if (ret != BF_SUCCESS) {
        BFDEBUG("common_destroy_vm failed: %llx\n", ret);
        return BF_IOCTL_FAILURE;
    }

    BFDEBUG("IOCTL_DESTROY_VM: succeeded\n");
    return BF_IOCTL_SUCCESS;
}

NTSTATUS
builderQueueInitialize(
    _In_ WDFDEVICE Device
)
{
    WDFQUEUE queue;
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG queueConfig;

    platform_init();

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
        &queueConfig,
        WdfIoQueueDispatchParallel
    );

    queueConfig.EvtIoStop = builderEvtIoStop;
    queueConfig.EvtIoDeviceControl = builderEvtIoDeviceControl;

    status = WdfIoQueueCreate(Device, &queueConfig, WDF_NO_OBJECT_ATTRIBUTES, &queue);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    BFDEBUG("builderQueueInitialize: success\n");
    return STATUS_SUCCESS;
}

VOID
builderEvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
)
{
    PVOID in = 0;
    PVOID out = 0;
    size_t in_size = 0;
    size_t out_size = 0;

    int64_t ret = 0;
    NTSTATUS status;

    UNREFERENCED_PARAMETER(Queue);

    if (InputBufferLength != 0) {
        status = WdfRequestRetrieveInputBuffer(Request, InputBufferLength, &in, &in_size);

        if (!NT_SUCCESS(status)) {
            goto IOCTL_FAILURE;
        }
    }

    if (OutputBufferLength != 0) {
        status = WdfRequestRetrieveOutputBuffer(Request, OutputBufferLength, &out, &out_size);

        if (!NT_SUCCESS(status)) {
            goto IOCTL_FAILURE;
        }
    }

    switch (IoControlCode) {
        case IOCTL_CREATE_VM:
            if (!in) {
                BFDEBUG("IOCTL_CREATE_VM: in buffer is NULL\n");
                goto IOCTL_FAILURE;
            }

            if (!out) {
                BFDEBUG("IOCTL_CREATE_VM: out buffer is NULL\n");
                goto IOCTL_FAILURE;
            }

            ret = ioctl_create_vm((struct create_vm_args *)in);
            RtlCopyMemory(out, in, (out_size > in_size) ? in_size : out_size);
            break;

        case IOCTL_DESTROY_VM:
            if (!in) {
                BFDEBUG("IOCTL_DESTROY_VM: in buffer is NULL\n");
                goto IOCTL_FAILURE;
            }

            ret = ioctl_destroy_vm((domainid_t *)in);
            break;

        default:
            goto IOCTL_FAILURE;
    }

    if (OutputBufferLength != 0) {
        WdfRequestSetInformation(Request, out_size);
    }

    if (ret != BF_IOCTL_SUCCESS) {
        goto IOCTL_FAILURE;
    }

    WdfRequestComplete(Request, STATUS_SUCCESS);
    return;

IOCTL_FAILURE:

    WdfRequestComplete(Request, STATUS_ACCESS_DENIED);
    return;
}

VOID
builderEvtIoStop(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ ULONG ActionFlags
)
{
    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(ActionFlags);

    WdfRequestComplete(Request, STATUS_SUCCESS);
    return;
}