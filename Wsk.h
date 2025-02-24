#pragma once

#ifndef __SOCKET_H
#define __SOCKET_H

#include "../include/Global.h"
#include <wsk.h>
#pragma comment(lib, "ws2_32.lib")

#define PMARKER

#define HTONS(n) (USHORT)(((n & 0xFF) << 8) | \
    ((n & 0xFF00) >> 8))

#define IP_ADDR(a, b, c, d) \
((d<<24) | \
    (c<<16) | \
        (b<<8) | a)


typedef struct _WSK_APP_SOCKET_CONTEXT {
    PWSK_SOCKET                 Socket;
} WSK_APP_SOCKET_CONTEXT, PMARKER* PWSK_APP_SOCKET_CONTEXT;

WSK_APP_SOCKET_CONTEXT          SockContext;
WSK_PROVIDER_NPI                WSKProviderNPI;
WSK_REGISTRATION                WSKRegistration;

NTSTATUS NTAPI
IoCompletionRoutine(
    _In_ PDEVICE_OBJECT         DeviceObject,
    _In_ PIRP                   Irp,
    _In_ PVOID                  Context
);

NTSTATUS NTAPI
WSKStartup(
    _In_ PWSK_REGISTRATION      WskRegistration
);

VOID NTAPI
WSKCleanup(
    _In_ PWSK_REGISTRATION      WskRegistration
);

NTSTATUS NTAPI
WSKSocket(
    _In_ PWSK_APP_SOCKET_CONTEXT SocketContext,
    _In_ ADDRESS_FAMILY         AddressFamily,
    _In_ USHORT                 SocketType,
    _In_ ULONG                  Protocol,
    _In_ ULONG                  Flags
);

NTSTATUS NTAPI
WSKBindSocket(
    _In_ PWSK_SOCKET            Socket,
    _In_ SOCKADDR_IN            LocalAddress,
    _In_ ULONG                  Flags
);

NTSTATUS NTAPI
WSKConnectSocket(
    _In_ PWSK_SOCKET            Socket,
    _In_ SOCKADDR_IN            RemoteAddress,
    _In_ ULONG                  Flags
);

NTSTATUS NTAPI
WSKDisconnectSocket(
    _In_ PWSK_SOCKET            Socket,
    _In_ ULONG                  Flags
);

NTSTATUS NTAPI
WSKCloseSocket(
    _In_ PWSK_SOCKET            Socket
);

NTSTATUS NTAPI
WSKAcceptConnection(
    _In_ PWSK_SOCKET            Socket,
    _In_opt_ PVOID              Context,
    _In_opt_ const WSK_CLIENT_CONNECTION_DISPATCH* Dispatch,
    _Out_opt_ SOCKADDR_IN       LocalAddress,
    _Out_opt_ SOCKADDR_IN       RemoteAddress,
    _In_ ULONG                  Flags
);

NTSTATUS NTAPI
WSKSendData(
    _In_ PWSK_SOCKET            Socket,
    _In_ PWSK_BUF               Buffer,
    _In_ ULONG                  Flags
);

NTSTATUS NTAPI
WSKReceiveData(
    _In_ PWSK_SOCKET            Socket,
    _In_ PWSK_BUF               Buffer,
    _In_ ULONG                  Flags
);


#endif // __SOCKET_H
