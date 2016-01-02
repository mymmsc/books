// Module Name: printqos.c
//
// Description:
//    This file contains routines used to print out the QOS data
//    structure from top to bottom. This includes the provider
//    specific structures. This file contains only support routines.
//
// Compile:
//    cl /c printqos.c
//
#include <winsock2.h>
#include <windows.h>

#include <qos.h>
#include <qossp.h>

#include "printqos.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_INDENT        128

//
// Function: PrintQos
//
// Description:
//    This is the top level function. It prints out the sending and
//    receiving FLOWSPEC structures and initiates printing of the 
//    provider specific data.
//  
void PrintQos(QOS *pqos)
{
    if (pqos == NULL)
        return;
    printf("Sending Flowspec:\n");
    PrintFlowspec(&pqos->SendingFlowspec, 1);
    printf("Receiving Flowspec:\n");
    PrintFlowspec(&pqos->ReceivingFlowspec, 1);
    printf("Provider Specific (len = %d bytes):\n", pqos->ProviderSpecific.len);
    PrintProviderSpecific(&pqos->ProviderSpecific, 1);
    printf("\n\n");
}

//
// Function: PrintFlowspec
//
// Description:
//    Prints the FLOWSPEC structure.
//
void PrintFlowspec(FLOWSPEC *pflow, int indent)
{
    char    szIndent[MAX_INDENT];

    memset(szIndent, ' ', MAX_INDENT);
    szIndent[indent * 3] = 0;

    if (pflow->TokenRate == QOS_NOT_SPECIFIED)
        printf("%sTokenRate          = QOS_NOT_SPECIFIED\n", szIndent);
    else
        printf("%sTokenRate          = %d bytes/sec\n", szIndent, pflow->TokenRate);
    if (pflow->TokenBucketSize == QOS_NOT_SPECIFIED)
        printf("%sTokenBucketSize    = QOS_NOT_SPECIFIED\n", szIndent);
    else
        printf("%sTokenBucketSize    = %d bytes\n", szIndent, pflow->TokenBucketSize);
    if (pflow->PeakBandwidth == QOS_NOT_SPECIFIED)
        printf("%sPeakBandwidth      = QOS_NOT_SPECIFIED\n", szIndent);
    else
        printf("%sPeakBandwidth      = %d bytes/sec\n", szIndent, pflow->PeakBandwidth);
    if (pflow->Latency == QOS_NOT_SPECIFIED)
        printf("%sLatency            = QOS_NOT_SPECIFIED\n", szIndent);
    else
        printf("%sLatency            = %d microseconds\n", szIndent, pflow->Latency);
    if (pflow->DelayVariation == QOS_NOT_SPECIFIED)
        printf("%sDelayVariation     = QOS_NOT_SPECIFIED\n", szIndent);
    else
        printf("%sDelayVariation     = %d microseconds\n", szIndent, pflow->DelayVariation);
 
    printf("%sServiceType        = %s\n", szIndent, GetServiceTypeStr(pflow->ServiceType));

    if (pflow->MaxSduSize == QOS_NOT_SPECIFIED)
        printf("%sMaxSduSize         = QOS_NOT_SPECIFIED\n", szIndent);
    else
        printf("%sMaxSduSize         = %d bytes\n", szIndent, pflow->MaxSduSize);
    if (pflow->MinimumPolicedSize == QOS_NOT_SPECIFIED)
        printf("%sMinimumPolicedSize = QOS_NOT_SPECIFIED\n", szIndent);
    else
        printf("%sMinimumPolicedSize = %d bytes\n", szIndent, pflow->MinimumPolicedSize);
}

//
// Function: PrintProviderSpecific
//
// Description:
//    Prints provider specific data. This is done by looking at
//    the WSABUF structure.  If the len is 0 there is no data.
//    Otherwise, the first structure in the data is a QOS_OBJECT_HDR
//    so we can cast the pointer as this object and find out 
//    specifically what object it is. Each header has its length
//    so we can find the start of the next QOS_OBJECT_HDR.
//
static void PrintProviderSpecific(WSABUF *provider, int indent)
{
    QOS_OBJECT_HDR  *objhdr=NULL;
    char            *bufptr=NULL,
                     szIndent[MAX_INDENT];
    BOOL             bDone = FALSE;
    DWORD            objcount=0;            

    memset(szIndent, ' ', MAX_INDENT);
    szIndent[indent * 3] = 0;
    //
    // If the buffer is empy, exit
    //
    if (provider->len == 0)
    {
        printf("%sEmpty Provider Specific\n", szIndent);
        return;
    }
    // Setup some pointers to the first object header
    //
    bufptr = (char *)provider->buf;
    objhdr = (QOS_OBJECT_HDR *)bufptr;
    //
    // Loop until we run out of objects
    //
    while (!bDone)
    {
        // Decode which object is present and call the appropriate
        // print routine.
        //
        switch (objhdr->ObjectType)
        {
            case RSVP_OBJECT_STATUS_INFO:
                PrintRsvpStatus((RSVP_STATUS_INFO *)objhdr, indent+1);
                break;
            case RSVP_OBJECT_RESERVE_INFO:
                PrintRsvpResv((RSVP_RESERVE_INFO *)objhdr, indent+1);
                break;
            case RSVP_OBJECT_ADSPEC:
                PrintRsvpAdspec((RSVP_ADSPEC *)objhdr, indent+1);
                break;
            case RSVP_OBJECT_POLICY_INFO:
                PrintRsvpPolicy((RSVP_POLICY_INFO *)objhdr, indent+1);
                break;
            case QOS_OBJECT_PRIORITY:
                PrintQosPriority((QOS_PRIORITY *)objhdr, indent+1);
                break;
            case QOS_OBJECT_SD_MODE:
                PrintQosSDMode((QOS_SD_MODE *)objhdr, indent+1);
                break;
            case QOS_OBJECT_TRAFFIC_CLASS:
                PrintQosTrafficClass((QOS_TRAFFIC_CLASS *)objhdr, indent+1);
                break;
            case QOS_OBJECT_DESTADDR:
                PrintQosDestAddr((QOS_DESTADDR *)objhdr, indent+1);
                break;
            case QOS_OBJECT_END_OF_LIST:
                bDone = TRUE;
                break;
            default:
                break;
        }
        // Update the pointers to the next object
        //
        bufptr += objhdr->ObjectLength;
        objcount += objhdr->ObjectLength;
        objhdr = (QOS_OBJECT_HDR *)bufptr;
        //
        // Check to see if we've exceeded the length of the object
        //
        if (objcount >= provider->len)
            bDone = TRUE;
    }
    return;
}

// 
// Function: GetServiceTypeStr
//
// Description:
//    This function is used by the PrintFlowspec call. It simply
//    returns a string for the ServiceType as opposed to printing
//    the integer value.
//
static char *GetServiceTypeStr(SERVICETYPE type)
{
    static char szServiceType[SERVICETYPE_STR_LEN];

    if (type == NULL_QOS_TYPE)
    {
        strcpy(szServiceType, "NULL_QOS_TYPE");
        return szServiceType;
    }
    if ((type & SERVICETYPE_BESTEFFORT) == SERVICETYPE_BESTEFFORT)
        strcpy(szServiceType, "SERVICETYPE_BESTEFFORT");
    else if ((type & SERVICETYPE_CONTROLLEDLOAD) == SERVICETYPE_CONTROLLEDLOAD)
        strcpy(szServiceType, "SERVICETYPE_CONTROLLEDLOAD");
    else if ((type & SERVICETYPE_GUARANTEED) == SERVICETYPE_GUARANTEED)
        strcpy(szServiceType, "SERVICETYPE_GUARANTEED");
    else if ((type & SERVICETYPE_NETWORK_UNAVAILABLE) == SERVICETYPE_NETWORK_UNAVAILABLE)
        strcpy(szServiceType, "SERVICETYPE_NETWORK_UNAVAILABLE");
    else if ((type & SERVICETYPE_GENERAL_INFORMATION) == SERVICETYPE_GENERAL_INFORMATION)
        strcpy(szServiceType, "SERVICETYPE_GENERAL_INFORMATION");
    else if ((type & SERVICETYPE_NOCHANGE) == SERVICETYPE_NOCHANGE)
        strcpy(szServiceType, "SERVICETYPE_NOCHANGE");
    else if ((type & SERVICETYPE_NONCONFORMING) == SERVICETYPE_NONCONFORMING)
        strcpy(szServiceType, "SERVICETYPE_NONCONFORMING");
    else if ((type & SERVICETYPE_NOTRAFFIC) == SERVICETYPE_NOTRAFFIC)
        strcpy(szServiceType, "SERVICETYPE_NOTRAFFIC");
    else
        strcpy(szServiceType, "Unknown");

    if ((type & SERVICE_NO_TRAFFIC_CONTROL) == SERVICE_NO_TRAFFIC_CONTROL)
        strcat(szServiceType, " | SERVICE_NO_TRAFFIC_CONTROL");
    else if ((type & SERVICE_NO_QOS_SIGNALING) == SERVICE_NO_QOS_SIGNALING)
        strcat(szServiceType, " | SERVICE__NO_QOS_SIGNALING");
    return szServiceType;
}

//
// Function: PrintRsvpStatus
//
// Description:
//    Prints the RSVP_STATUS_INFO object.
//
static void PrintRsvpStatus(RSVP_STATUS_INFO *status, int indent)
{
    char    szIndent[MAX_INDENT];

    memset(szIndent, ' ', MAX_INDENT);
    szIndent[indent * 3] = 0;

    printf("%sStatus Code = ", szIndent);
    switch (status->StatusCode)
    {
        case WSA_QOS_RECEIVERS:             // at least one RESV has arrived 
            printf("WSA_QOS_RECEIVERS\n"); 
            break;
        case WSA_QOS_SENDERS:               // at least one PATH has arrived 
            printf("WSA_QOS_SENDERS\n"); 
            break;
        case WSA_QOS_NO_SENDERS:            // there are no senders
            printf("WSA_QOS_NO_SENDERS\n"); 
            break;
        case WSA_QOS_NO_RECEIVERS:          // there are no receivers
            printf("WSA_QOS_NO_RECEIVERS\n"); 
            break;
        case WSA_QOS_REQUEST_CONFIRMED:     // Reserve has been confirmed
            printf("WSA_QOS_REQUEST_CONFIRMED\n"); 
            break;
        case WSA_QOS_ADMISSION_FAILURE:     // error due to lack of resources
            printf("WSA_QOS_ADMISSION_FAILURE\n"); 
            break;
        case WSA_QOS_POLICY_FAILURE:        // rejected for admin reasons
            printf("WSA_QOS_POLICY_FAILURE\n"); 
            break;
        case WSA_QOS_BAD_STYLE:             // unknown or conflicting style
            printf("WSA_QOS_BAD_STYLE\n"); 
            break;
        case WSA_QOS_BAD_OBJECT:            // problem with some part of the 
                                            // filterspec/providerspecific 
                                            // buffer in general 
            printf("WSA_QOS_BAD_OBJECT\n"); 
            break;
        case WSA_QOS_TRAFFIC_CTRL_ERROR:    // problem with some part of the 
                                            // flowspec
            printf("WSA_QOS_TRAFFIC_CTRL_ERROR\n"); 
            break;
        case WSA_QOS_GENERIC_ERROR:         // general error 
            printf("WSA_QOS_GENERIC_ERROR\n");
            break;
        default:
            printf("Unknown RSVP StatusCode %lu\n", status->StatusCode); 
            break;
    }
    printf("%sExtendedStatus1=%lu\n", szIndent, status->ExtendedStatus1); 
    printf("%sExtendedStatus2=%lu\n", szIndent, status->ExtendedStatus2); 

    return;
}

//
// Function: PrintRsvpResv
//
// Description:
//    Prints the RSVP_RESERVE_INFO object.
//
static void PrintRsvpResv(RSVP_RESERVE_INFO *resv, int indent)
{
    SOCKADDR_IN   addr;
    DWORD         dwIPv6Len,
                  i, j;
    char          szIPv6[128],
                  szIndent[MAX_INDENT];

    memset(szIndent, ' ', MAX_INDENT);
    szIndent[indent * 3] = 0;

    printf("%sStye = ", szIndent);
    switch (resv->Style)                // filter style
    {
        case RSVP_DEFAULT_STYLE:
            // Use the default for this socket type 
            printf("RSVP_DEFAULT_STYLE\n");
            break;
        case RSVP_WILDCARD_STYLE:
            printf("RSVP_WILDCARD_STYLE\n");
            break;
        case RSVP_FIXED_FILTER_STYLE:
            printf("RSVP_FIXED_FILTER_STYLE\n");
            break;
        case RSVP_SHARED_EXPLICIT_STYLE:
            printf("RSVP_SHARED_EXPLICIT_STYLE\n");
            break;
        default:
            printf("Unknown style %d\n", resv->Style);
            break;
    }
    printf("%sConfirmRequest = %s\n", szIndent, (resv->ConfirmRequest ? "yes":"no"));
    PrintPolicy(resv->PolicyElementList, resv->NumPolicyElement, indent+1);
    printf("%sNumFlowDesc = %lu\n", szIndent, resv->NumFlowDesc);
    //
    // Print out the flow descriptors (if any) associated with the filter
    //
    for(i=0; i < resv->NumFlowDesc ;i++)
    {
        PrintFlowspec(&resv->FlowDescList[i].FlowSpec, indent+1);
        printf("%s  NumFilters = %lu\n", szIndent, resv->FlowDescList[i].NumFilters);
        for(j=0; j < resv->FlowDescList[i].NumFilters ;j++)
        {
            printf("%s  FilterType = ", szIndent);
            switch (resv->FlowDescList[i].FilterList[j].Type)
            {
                case FILTERSPECV4:
                    addr.sin_addr.s_addr = resv->FlowDescList[i].FilterList[j].FilterSpecV4.Address.Addr;
                    printf("%s    FILTERSPECV4\n", szIndent);
                    printf("%s      Address = %s\n", szIndent, inet_ntoa(addr.sin_addr));
                    printf("%s      Port    = %d\n", szIndent,
                        ntohs(resv->FlowDescList[i].FilterList[j].FilterSpecV4.Port));
                    break;
                case FILTERSPECV6:
                    printf("%s    FILTERSPECV6\n", szIndent);
                    WSAAddressToString(
                        (LPSOCKADDR)&resv->FlowDescList[i].FilterList[j].FilterSpecV6.Address,
                        sizeof(IN_ADDR_IPV6),
                        NULL,
                        szIPv6,
                        &dwIPv6Len);
                    printf("%s      Address = %s\n", szIndent, szIPv6);
                    printf("%s      Port    = %hu\n", szIndent, 
                        resv->FlowDescList[i].FilterList[j].FilterSpecV6.Port);
                    break;
                case FILTERSPECV6_FLOW:
                    printf("%s    FILTERSPECV6_FLOW\n", szIndent);
                    WSAAddressToString(
                        (LPSOCKADDR)&resv->FlowDescList[i].FilterList[j].FilterSpecV6Flow.Address,
                        sizeof(IN_ADDR_IPV6),
                        NULL,
                        szIPv6,
                        &dwIPv6Len);
                    printf("%s      Address = %s\n", szIndent, szIPv6);
                    printf("%s      FlowLabel = %s\n", szIndent, 
                        resv->FlowDescList[i].FilterList[j].FilterSpecV6Flow.FlowLabel);
                    break;
                case FILTERSPECV4_GPI:
                    addr.sin_addr.s_addr = resv->FlowDescList[i].FilterList[j].FilterSpecV4Gpi.Address.Addr;
                    printf("%s    FILTERSPECV4_GPI\n", szIndent);
                    printf("%s      Address = %s\n", szIndent, inet_ntoa(addr.sin_addr));
                    printf("%s      Port    = %d\n", szIndent, 
                        resv->FlowDescList[i].FilterList[j].FilterSpecV4Gpi.GeneralPortId);
                    break;
                case FILTERSPECV6_GPI:
                    printf("%s    FILTERSPECV6_GPI\n", szIndent);
                    WSAAddressToString(
                        (LPSOCKADDR)&resv->FlowDescList[i].FilterList[j].FilterSpecV6Gpi.Address,
                        sizeof(IN_ADDR_IPV6),
                        NULL,
                        szIPv6,
                        &dwIPv6Len);
                    printf("%s      Address = %s\n", szIndent, szIPv6);
                    printf("%s      Port    = %hu\n", szIndent,
                        resv->FlowDescList[i].FilterList[j].FilterSpecV6Gpi.GeneralPortId);
                    break;
            }
        }
    }
    return;
}

//
// Function: PrintRsvpAdspec
//
// Description:
//    Prints the RSVP_ADSPEC object.
//
static void PrintRsvpAdspec(RSVP_ADSPEC *adspec, int indent)
{
    DWORD   i;
    char    szIndent[MAX_INDENT];

    memset(szIndent, ' ', MAX_INDENT);
    szIndent[indent * 3] = 0;

    printf("%sGeneralParams\n", szIndent);
    PrintAdGeneralParams(&adspec->GeneralParams, indent+1);

    printf("%sNumberOfServices = %d\n", szIndent, adspec->NumberOfServices);
    for(i=0; i < adspec->NumberOfServices ;i++)
    {
        printf("%s  Services[%d].Length = %lu\n", szIndent, 
            i, adspec->Services[i].Length);
        printf("%s  Services[%d].ServiceType = %s\n", szIndent,
            i, GetServiceTypeStr(adspec->Services[i].Service));
        printf("%s  Services[%d].Overrides\n",  szIndent, i);

        PrintAdGeneralParams(&adspec->Services[i].Overrides, indent+1);

        if (adspec->Services[i].Service == SERVICETYPE_GUARANTEED)
        {
            printf("%s     Services[%lu].CTotal = %lu\n", szIndent, i, 
                adspec->Services[i].Guaranteed.CTotal);
            printf("%s     Services[%lu].DTotal = %lu\n", szIndent, i, 
                adspec->Services[i].Guaranteed.DTotal);
            printf("%s     Services[%lu].CSum   = %lu\n", szIndent, i, 
                adspec->Services[i].Guaranteed.CSum);
            printf("%s     Services[%lu].DSum   = %lu\n", szIndent, i, 
                adspec->Services[i].Guaranteed.DSum);
        }
        else
        {
            printf("%s     Services[%lu].ParamBuffer[].ParameterId = %lu\n",
                szIndent, i, adspec->Services[i].ParamBuffer->ParameterId);
            printf("%s     Services[%lu].ParamBuffer[].Length      = %lu\n",
                szIndent, i, adspec->Services[i].ParamBuffer->Length);
        }
    }
    return;
}

//
// Function: PrintPolicy
//
// Description:
//    Print RSVP_POLICY objects.
//
static void PrintPolicy(RSVP_POLICY *pelements, int num, int indent)
{
    int      i;
    char     szIndent[MAX_INDENT];

    memset(szIndent, ' ', MAX_INDENT);
    szIndent[indent * 3] = 0;

    printf("%sNum Policy Elements = %d\n", szIndent, num);
    for(i=0; i < num ;i++)
    {
        printf("%s  Len  = %hu\n", szIndent, pelements[i].Len);
        printf("%s  Type = %hu\n", szIndent, pelements[i].Type);
    }
    return;
}

//
// Fucnction: PrintAdGeneralParams
//
// Description:
//    Print AD_GENERAL_PARAMS object.
//
static void PrintAdGeneralParams(AD_GENERAL_PARAMS *params, int indent)
{
    char    szIndent[MAX_INDENT];

    memset(szIndent, ' ', MAX_INDENT);
    szIndent[indent * 3] = 0;

    printf("%sIntServAwareHopCount  = 0x%X\n", szIndent, params->IntServAwareHopCount);
    if (params->PathBandwidthEstimate == POSITIVE_INFINITY_RATE)
        printf("%sPathBandwidthEstimate = POSITIVE INFINITY\n", szIndent);
    else
        printf("%sPathBandwidthEstimate = 0x%X\n", szIndent, params->PathBandwidthEstimate);
    printf("%sMinimumLatency        = 0x%X\n", szIndent, params->MinimumLatency);
    printf("%sPathMTU               = 0x%X\n", szIndent, params->PathMTU);
    printf("%sFlags                 = %lu\n", szIndent, params->Flags);

    return;
}

//
// Function: PrintRsvpPolicy
//
// Description:
//    Print the RSVP_POLICY_INFO object.
//
static void PrintRsvpPolicy(RSVP_POLICY_INFO *policy, int indent)
{
    PrintPolicy(policy->PolicyElement, policy->NumPolicyElement, indent+1);
    return;
}

//
// Function: PrintQosPriority
//
// Description:
//    Print the QOS_PRIORITY object.
//
static void PrintQosPriority(QOS_PRIORITY *priority, int indent)
{
    char    szIndent[MAX_INDENT];

    memset(szIndent, ' ', MAX_INDENT);
    szIndent[indent * 3] = 0;

    printf("%sSendPriority    = %u\n", szIndent, priority->SendPriority);
    printf("%sSendFlags       = %u\n", szIndent, priority->SendFlags);
    printf("%sReceivePriority = %u\n", szIndent, priority->ReceivePriority);
    return;
}

//
// Function: PrintQosSDMode
//
// Description:
//    Print the QOS_SD_MODE object.
//
static void PrintQosSDMode(QOS_SD_MODE *sd, int indent)
{
    char    szIndent[MAX_INDENT];

    memset(szIndent, ' ', MAX_INDENT);
    szIndent[indent * 3] = 0;

    printf("%sShapeDiscard Mode = ", szIndent);
    switch (sd->ShapeDiscardMode)
    {
        case TC_NONCONF_BORROW:       // use best efford for non conforming
            printf("TC_NONCONF_BORROW\n"); 
            break;
        case TC_NONCONF_SHAPE:        // hold data until it conforms
            printf("TC_NONCONF_SHAPE\n"); 
            break;
        case TC_NONCONF_DISCARD:      // drop any non-conforming data
            printf("TC_NONCONF_DISCARD\n"); 
            break;
        default:
            printf("Unknown traffic class %lu\n", sd->ShapeDiscardMode);
            break;
    }
    return;
}

//
// Function: PrintQosTrafficClass
//
// Description:
//    Print the QOS_TRAFFIC_CLASS object.
//
static void PrintQosTrafficClass(QOS_TRAFFIC_CLASS *traffic, int indent)
{
    char    szIndent[MAX_INDENT];

    memset(szIndent, ' ', MAX_INDENT);
    szIndent[indent * 3] = 0;
    printf("%sTrafficClass = %lu\n", szIndent, traffic->TrafficClass);
    return;
}

//
// Function: PrintQosDestAddr
//
// Description:
//    Print the QOS_DESTADDR structure.
//
static void PrintQosDestAddr(QOS_DESTADDR *dest, int indent)
{
    char    szIndent[MAX_INDENT];

    memset(szIndent, ' ', MAX_INDENT);
    szIndent[indent * 3] = 0;

    printf("%sDestAddress     = %s\n", szIndent, inet_ntoa(((SOCKADDR_IN *)dest->SocketAddress)->sin_addr));
    printf("%sDestAddress Len = %lu\n", szIndent, dest->SocketAddressLength);
    return;
}
