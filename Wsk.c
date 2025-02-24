#include "Socket.h"

const WSK_CLIENT_DISPATCH WSKAppDispatch = {
    MAKE_WSK_VERSION(1, 0),
    0,
    NULL
};

NTSTATUS NTAPI
IoCompletionRoutine(
    _In_ PDEVICE_OBJECT         DeviceObject,
    _In_ PIRP                   Irp,
    _In_ PVOID                  Context
) {
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    KeSetEvent(
        (PKEVENT)Context,
        IO_NO_INCREMENT,
        FALSE
    );

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS NTAPI
WSKStartup(
    _In_ PWSK_REGISTRATION      WskRegistration
) {
    NTSTATUS                    Status;
    WSK_CLIENT_NPI              WSKClientNpi = { 0 };

    WSKClientNpi.ClientContext = NULL;
    WSKClientNpi.Dispatch = &WSKAppDispatch;

    Status = WskRegister(
        &WSKClientNpi,
        WskRegistration
    );

    if (NT_SUCCESS(Status) == FALSE)
        return Status;

    Status = WskCaptureProviderNPI(
        WskRegistration,
        15,
        &WSKProviderNPI
    );

    return Status;
}

VOID NTAPI
WSKCleanup(
    _In_ PWSK_REGISTRATION      WskRegistration
) {
    WskDeregister(
        WskRegistration
    );

    WskReleaseProviderNPI(
        WskRegistration
    );

}

NTSTATUS NTAPI
WSKSocket(
    _In_ PWSK_APP_SOCKET_CONTEXT SocketContext,
    _In_ ADDRESS_FAMILY         AddressFamily,
    _In_ USHORT                 SocketType,
    _In_ ULONG                  Protocol,
    _In_ ULONG                  Flags
) {
    NTSTATUS                    Status;
    PIRP                        IRP;
    KEVENT                      Event;

    KeInitializeEvent(
        &Event,
        NotificationEvent,
        FALSE
    );

    IRP = IoAllocateIrp(
        1,
        FALSE
    );

    if (IRP == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IoSetCompletionRoutine(
        IRP,
        IoCompletionRoutine,
        &Event,
        TRUE, TRUE, TRUE
    );

    Status = WSKProviderNPI.Dispatch->WskSocket(
        WSKProviderNPI.Client,
        AddressFamily,
        SocketType,
        Protocol,
        Flags,
        NULL, NULL, NULL, NULL, NULL,
        IRP
    );

    if (NT_SUCCESS(Status) == FALSE) {
        IoFreeIrp(IRP);
        return Status;
    }

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(
            &Event,
            Executive,
            KernelMode,
            FALSE,
            NULL
        );
    }

    Status = IRP->IoStatus.Status;
    SocketContext->Socket = (PWSK_SOCKET)IRP->IoStatus.Information;

    IoFreeIrp(IRP);

    return Status;
}

NTSTATUS NTAPI
WSKBindSocket(
    _In_ PWSK_SOCKET            Socket,
    _In_ SOCKADDR_IN            LocalAddress,
    _In_ ULONG                  Flags
) {
    NTSTATUS                    Status;
    PIRP                        IRP;
    KEVENT                      Event;

    PWSK_PROVIDER_CONNECTION_DISPATCH Dispatch = (PWSK_PROVIDER_CONNECTION_DISPATCH)Socket->Dispatch;


    KeInitializeEvent(
        &Event,
        NotificationEvent,
        FALSE
    );

    IRP = IoAllocateIrp(
        1,
        FALSE
    );

    if (IRP == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IoSetCompletionRoutine(
        IRP,
        IoCompletionRoutine,
        &Event,
        TRUE, TRUE, TRUE
    );


    Status = Dispatch->WskBind(
        Socket,
        (PSOCKADDR)&LocalAddress,
        Flags,
        IRP
    );

    if (NT_SUCCESS(Status) == FALSE) {
        IoFreeIrp(IRP);
        return Status;
    }

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(
            &Event,
            Executive,
            KernelMode,
            FALSE,
            NULL
        );
    }

    Status = IRP->IoStatus.Status;
    IoFreeIrp(IRP);

    return Status;
}

NTSTATUS NTAPI
WSKConnectSocket(
    _In_ PWSK_SOCKET            Socket,
    _In_ SOCKADDR_IN            RemoteAddress,
    _In_ ULONG                  Flags
) {
    NTSTATUS                    Status;
    PIRP                        IRP;
    KEVENT                      Event;

    PWSK_PROVIDER_CONNECTION_DISPATCH Dispatch = (PWSK_PROVIDER_CONNECTION_DISPATCH)(Socket->Dispatch);

    KeInitializeEvent(
        &Event,
        NotificationEvent,
        FALSE
    );


    IRP = IoAllocateIrp(
        1,
        FALSE
    );

    if (IRP == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    IoSetCompletionRoutine(
        IRP,
        IoCompletionRoutine,
        &Event,
        TRUE, TRUE, TRUE
    );


    Status = Dispatch->WskConnect(
        Socket,
        (PSOCKADDR)&RemoteAddress,
        Flags,
        IRP
    );

    if (NT_SUCCESS(Status) == FALSE) {
        IoFreeIrp(IRP);
        return Status;
    }

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(
            &Event,
            Executive,
            KernelMode,
            FALSE,
            NULL
        );
    }

    Status = IRP->IoStatus.Status;
    IoFreeIrp(IRP);

    return Status;
}

NTSTATUS NTAPI
WSKDisconnectSocket(
    _In_ PWSK_SOCKET            Socket,
    _In_ ULONG                  Flags
) {
    NTSTATUS                    Status;
    PIRP                        IRP;
    KEVENT                      Event;

    PWSK_PROVIDER_CONNECTION_DISPATCH Dispatch = (PWSK_PROVIDER_CONNECTION_DISPATCH)(Socket->Dispatch);

    KeInitializeEvent(
        &Event,
        NotificationEvent,
        FALSE
    );


    IRP = IoAllocateIrp(
        1,
        FALSE
    );

    if (IRP == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    IoSetCompletionRoutine(
        IRP,
        IoCompletionRoutine,
        &Event,
        TRUE, TRUE, TRUE
    );


    Status = Dispatch->WskDisconnect(
        Socket,
        NULL,
        Flags,
        IRP
    );

    if (NT_SUCCESS(Status) == FALSE) {
        IoFreeIrp(IRP);
        return Status;
    }

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(
            &Event,
            Executive,
            KernelMode,
            FALSE,
            NULL
        );
    }

    Status = IRP->IoStatus.Status;
    IoFreeIrp(IRP);

    return Status;
}

NTSTATUS NTAPI
WSKSendData(
    _In_ PWSK_SOCKET            Socket,
    _In_ PWSK_BUF               Buffer,
    _In_ ULONG                  Flags
) {
    NTSTATUS                    Status;
    PIRP                        IRP;
    KEVENT                      Event;

    PWSK_PROVIDER_CONNECTION_DISPATCH Dispatch = (PWSK_PROVIDER_CONNECTION_DISPATCH)(Socket->Dispatch);

    MmBuildMdlForNonPagedPool(Buffer->Mdl);

    KeInitializeEvent(
        &Event,
        NotificationEvent,
        FALSE
    );


    IRP = IoAllocateIrp(
        1,
        FALSE
    );

    if (IRP == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    IoSetCompletionRoutine(
        IRP,
        IoCompletionRoutine,
        &Event,
        TRUE, TRUE, TRUE
    );


    Status = Dispatch->WskSend(
        Socket,
        Buffer,
        Flags,
        IRP
    );

    if (NT_SUCCESS(Status) == FALSE) {
        IoFreeMdl(Buffer->Mdl);
        IoFreeIrp(IRP);
        return Status;
    }

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(
            &Event,
            Executive,
            KernelMode,
            FALSE,
            NULL
        );
    }

    Status = IRP->IoStatus.Status;
    IoFreeMdl(Buffer->Mdl);
    IoFreeIrp(IRP);

    return Status;
}

NTSTATUS NTAPI
WSKReceiveData(
    _In_ PWSK_SOCKET            Socket,
    _Inout_ PWSK_BUF            Buffer,
    _In_ ULONG                  Flags
) {
    NTSTATUS                    Status;
    PIRP                        IRP;
    KEVENT                      Event;

    PWSK_PROVIDER_CONNECTION_DISPATCH Dispatch = (PWSK_PROVIDER_CONNECTION_DISPATCH)(Socket->Dispatch);

    MmBuildMdlForNonPagedPool(Buffer->Mdl);

    KeInitializeEvent(
        &Event,
        NotificationEvent,
        FALSE
    );

    IRP = IoAllocateIrp(
        1,
        FALSE
    );

    if (IRP == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IoSetCompletionRoutine(
        IRP,
        IoCompletionRoutine,
        &Event,
        TRUE, TRUE, TRUE
    );

    Status = Dispatch->WskReceive(
        Socket,
        Buffer,
        Flags,
        IRP
    );

    if (NT_SUCCESS(Status) == FALSE) {
        IoFreeMdl(Buffer->Mdl);
        IoFreeIrp(IRP);
        return Status;
    }

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(
            &Event,
            Executive,
            KernelMode,
            FALSE,
            NULL
        );
    }

    Status = IRP->IoStatus.Status;

    ULONG ReceivedLength = (ULONG)Buffer->Length;
    if (ReceivedLength < Buffer->Mdl->ByteCount) {
        ((CHAR*)Buffer->Mdl->MappedSystemVa)[ReceivedLength] = '\0';
    }

    IoFreeMdl(Buffer->Mdl);
    IoFreeIrp(IRP);

    return Status;
}

NTSTATUS NTAPI
WSKCloseSocket(
    _In_ PWSK_SOCKET            Socket
) {
    NTSTATUS                    Status;
    PIRP                        IRP;
    KEVENT                      Event;

    PWSK_PROVIDER_CONNECTION_DISPATCH Dispatch = (PWSK_PROVIDER_CONNECTION_DISPATCH)(Socket->Dispatch);

    KeInitializeEvent(
        &Event,
        NotificationEvent,
        FALSE
    );

    IRP = IoAllocateIrp(
        1,
        FALSE
    );

    if (IRP == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IoSetCompletionRoutine(
        IRP,
        IoCompletionRoutine,
        &Event,
        TRUE, TRUE, TRUE
    );

    Status = Dispatch->WskCloseSocket(
        Socket,
        IRP
    );

    if (NT_SUCCESS(Status) == FALSE) {
        IoFreeIrp(IRP);
        return Status;
    }

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(
            &Event,
            Executive,
            KernelMode,
            FALSE,
            NULL
        );
    }

    Status = IRP->IoStatus.Status;
    IoFreeIrp(IRP);

    return Status;
}
