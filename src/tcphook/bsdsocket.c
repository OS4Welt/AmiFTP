#include <utility/tagitem.h>
#include <sys/ioctl.h>
#include <proto/socket.h>
#include <proto/usergroup.h>
#include "tcphooks.h"

extern struct SocketIFace *ISocket;
extern struct UserGroupIFace *IUserGroup;

struct hostent *aos4_gethostbyname(const UBYTE *name)
{
    return ISocket->gethostbyname(name);
}

struct servent *aos4_getservbyname(const UBYTE *name, const UBYTE *proto)
{
    return ISocket->getservbyname(name,proto);
}

char *aos4_getlogin(void)
{
    return IUserGroup->getlogin();
}

struct passwd *aos4_getpwnam(const char *name)
{
    return IUserGroup->getpwnam(name);
}

void aos4_endpwent(void)
{
    IUserGroup->endpwent();
}

LONG aos4_gethostname(STRPTR hostname, LONG size)
{
    return ISocket->gethostname(hostname,size);
}

ULONG aos4_inetaddr(const UBYTE *s)
{
    return ISocket->inet_addr(s);
}

LONG aos4_socket(LONG domain, LONG type, LONG protocol)
{
    return ISocket->socket(domain,type,protocol);
}

LONG aos4_ioctl(LONG fd, ULONG request, char *argp)
{
    if (request==TCPFIONBIO)
      request=FIONBIO;
    else if (request==TCPFIOASYNC)
      request=FIOASYNC;
    return ISocket->IoctlSocket(fd, request, argp);
}

LONG aos4_connect(LONG s, const struct mysockaddr_in *sin)
{
    struct sockaddr_in hisctladdr;

    bzero((char *)&hisctladdr,sizeof(hisctladdr));
    hisctladdr.sin_family=sin->sin_family;
    hisctladdr.sin_addr.s_addr=sin->sin_addr.s_addr;
    hisctladdr.sin_port=sin->sin_port;

    return ISocket->connect(s,(struct sockaddr *)&hisctladdr,sizeof(hisctladdr));
}

LONG aos4_waitselect(LONG nfds,  fd_set *readfds, fd_set *writefds, fd_set *exeptfds,
		       struct timeval *timeout, ULONG *maskp)
{
    return ISocket->WaitSelect(nfds, readfds,writefds,exeptfds,timeout,maskp);
}

LONG aos4_getpeername(LONG s, struct sockaddr *name, LONG *namelen)
{
    return ISocket->getpeername(s,name,namelen);
}

LONG aos4_closesocket(LONG d)
{
    return ISocket->CloseSocket(d);
}

LONG aos4_getsockname(LONG s, struct sockaddr *name, LONG *namelen)
{
    return ISocket->getsockname(s,name,namelen);
}

LONG aos4_setsockopt(LONG s, LONG level, LONG optname, void *optval, LONG optlen)
{
    return ISocket->setsockopt(s,level,optname,optval,optlen);
}

LONG aos4_send(LONG s, const UBYTE *msg, LONG len, LONG flags)
{
    return ISocket->send(s,msg,len,flags);
}

LONG aos4_recv(LONG s, UBYTE *buf, LONG len, LONG flags)
{
    return ISocket->recv(s,buf,len,flags);
}

LONG aos4_bind(LONG s, const struct sockaddr *name, LONG namelen)
{
    return ISocket->bind(s,name,namelen);
}

LONG aos4_listen(LONG s, LONG backlog)
{
    return ISocket->listen(s,backlog);
}

LONG aos4_accept(LONG s, struct sockaddr *addr, LONG *addrlen)
{
    return ISocket->accept(s,addr,addrlen);
}

LONG aos4_shutdown(LONG s, LONG how)
{
    return ISocket->shutdown(s,how);
}
    
int SetupAOS4Hooks()
{
    if (ISocket==NULL) return 0;

    tcp_socket=aos4_socket;
    tcp_bind=aos4_bind;
    tcp_listen=aos4_listen;
    tcp_accept=aos4_accept;
    tcp_connect=aos4_connect;
    tcp_send=aos4_send;
    tcp_recv=aos4_recv;
    tcp_shutdown=aos4_shutdown;
    tcp_setsockopt=aos4_setsockopt;
    tcp_getsockname=aos4_getsockname;
    tcp_getpeername= aos4_getpeername;
    tcp_ioctl=aos4_ioctl;
    tcp_closesocket=aos4_closesocket;
    tcp_waitselect=aos4_waitselect;
    tcp_inetaddr=aos4_inetaddr;
    tcp_gethostbyname=aos4_gethostbyname;
    tcp_endpwent=aos4_endpwent;
    tcp_getpwnam=aos4_getpwnam;
    tcp_getlogin=aos4_getlogin;
    tcp_gethostname=aos4_gethostname;
    tcp_getservbyname=aos4_getservbyname;
    
    return 1;
}
