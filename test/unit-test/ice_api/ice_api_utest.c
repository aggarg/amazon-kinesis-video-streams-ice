/* Unity includes. */
#include "unity.h"
#include "catch_assert.h"

/* Standard includes. */
#include <stdlib.h>
#include <time.h>
#include <string.h>

/* API includes. */
#include "ice_api.h"

/* ===========================  EXTERN VARIABLES    =========================== */

/*
 * The priority is calculated for a host candidate where pCandidate.isPointToPoint = 1.
 */
#define   HOST_CANDIDATE_PRIORITY                       2113929471
/*
 * The priority is calculated for a host candidate where pCandidate.isPointToPoint = 0.
 */
#define   HOST_CANDIDATE_PRIORITY_MULTICAST             2130706431
#define   CRC32_POLYNOMIAL                              0xEDB88320

IceInitInfo_t initInfo;
IceCandidate_t localCandidateArray[ 10 ];
IceCandidate_t remoteCandidateArray[ 10 ];
IceCandidatePair_t candidatePairArray[ 100 ];
TransactionIdStore_t transactionIdStore[ 16 ];


/* ===========================  EXTERN FUNCTIONS   =========================== */

IceResult_t testRandomFxn( uint8_t * pDest,
                           size_t length )
{
    size_t i;

    /* Fill the buffer with a repeating pattern. */
    for( i = 0; i < length; i++ )
    {
        pDest[ i ] = ( uint8_t ) ( i % 256 );
    }

    return ICE_RESULT_OK;
}

/*-----------------------------------------------------------*/

IceResult_t testCrc32Fxn( uint32_t initialResult,
                          const uint8_t * pBuffer,
                          size_t bufferLength,
                          uint32_t * pCalculatedCrc32 )
{
    size_t i, j;
    uint32_t crc32 = initialResult;

    /* Calculate the CRC32 using a simple algorithm. */
    for( i = 0; i < bufferLength; i++ )
    {
        crc32 ^= pBuffer[ i ];

        for( j = 0; j < 8; j++ )
        {
            if( ( crc32 & 1 ) != 0 )
            {
                crc32 = ( crc32 >> 1 ) ^ CRC32_POLYNOMIAL;
            }
            else
            {
                crc32 = ( crc32 >> 1 );
            }
        }
    }

    /* Store the calculated CRC32 value. */
    *pCalculatedCrc32 = crc32;

    return ICE_RESULT_OK;
}

/*-----------------------------------------------------------*/

IceResult_t testHmacFxn( const uint8_t * pPassword,
                         size_t passwordLength,
                         const uint8_t * pBuffer,
                         size_t bufferLength,
                         uint8_t * pOutputBuffer,
                         uint16_t * pOutputBufferLength )
{
    /* Assume a fixed HMAC output length of 32 bytes (256-bit). */
    const uint16_t hmacLength = 32;
    uint16_t i;
    IceResult_t result = ICE_RESULT_OK;

    if( *pOutputBufferLength < hmacLength )
    {
        result = ICE_RESULT_BAD_PARAM;
    }

    if( result == ICE_RESULT_OK )
    {
        /* Calculate the HMAC using a simple algorithm. */
        for( i = 0; i < hmacLength; i++ )
        {
            pOutputBuffer[ i ] = pPassword[ i % passwordLength ] ^
                                 pBuffer[ i % bufferLength ];
        }

        /* Update the output buffer length. */
        *pOutputBufferLength = hmacLength;
    }

    return result;
}

/*
   The following function is used to Initialize the initInfo For each Test case.
 */
void Info_Init_For_Tests( void )
{
    initInfo.pLocalCandidatesArray = &( localCandidateArray[ 0 ] );
    initInfo.pRemoteCandidatesArray = &( remoteCandidateArray[ 0 ] );
    initInfo.pCandidatePairsArray = &( candidatePairArray[ 0 ] );
    initInfo.pStunBindingRequestTransactionIdStore = &( transactionIdStore[ 0 ] );
    initInfo.cryptoFunctions.randomFxn = testRandomFxn;
    initInfo.cryptoFunctions.crc32Fxn = testCrc32Fxn;
    initInfo.cryptoFunctions.hmacFxn = testHmacFxn;
    initInfo.creds.pLocalUsername = ( uint8_t * ) "localUsername";
    initInfo.creds.pLocalPassword = ( uint8_t * ) "localPassword";
    initInfo.creds.pRemoteUsername = ( uint8_t * )  "remoteUsername";
    initInfo.creds.pRemotePassword = ( uint8_t * ) "remotePassword";
    initInfo.creds.pCombinedUsername = ( uint8_t * ) "combinedUsername";
    initInfo.isControlling = 1;
}

void setUp( void )
{
    Info_Init_For_Tests();
}

void tearDown( void )
{
    memset( &( initInfo ),
            0,
            sizeof( initInfo ));

    // Explicitly set the pointers to NULL
    initInfo.pLocalCandidatesArray = NULL;
    initInfo.pRemoteCandidatesArray = NULL;
    initInfo.pCandidatePairsArray = NULL;
    initInfo.pStunBindingRequestTransactionIdStore = NULL;
}
/* ==============================  Test Cases  ============================== */

/**
 * @brief Validate ICE Init fail functionality for Bad Parameters.
 */
void test_iceInit_BadParams( void )
{

    IceContext_t context = { 0 };
    IceResult_t result;
    uint8_t localUsername[ 10 ];
    uint8_t localPassword[ 10 ];
    uint8_t remoteUsername[ 10 ];
    uint8_t remotePassword[ 10 ];


    result = Ice_Init( NULL,
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_BAD_PARAM,
                       result );

    result = Ice_Init( &( context ),
                       NULL );

    TEST_ASSERT_EQUAL( ICE_RESULT_BAD_PARAM,
                       result );

    initInfo.pLocalCandidatesArray = NULL;

    result = Ice_Init( &( context ),
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_BAD_PARAM,
                       result );

    initInfo.pLocalCandidatesArray = &( localCandidateArray[ 0 ] );
    initInfo.pRemoteCandidatesArray = NULL;

    result = Ice_Init( &( context ),
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_BAD_PARAM,
                       result );

    initInfo.pRemoteCandidatesArray = &( localCandidateArray[ 0 ] );
    initInfo.pCandidatePairsArray = NULL;

    result = Ice_Init( &( context ),
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_BAD_PARAM,
                       result );

    initInfo.pCandidatePairsArray = &( candidatePairArray[ 0 ] );
    initInfo.pStunBindingRequestTransactionIdStore = NULL;

    result = Ice_Init( &( context ),
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_BAD_PARAM,
                       result );

    initInfo.pStunBindingRequestTransactionIdStore = &( transactionIdStore[ 0 ] );
    initInfo.cryptoFunctions.randomFxn = NULL;

    result = Ice_Init( &( context ),
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_BAD_PARAM,
                       result );

    initInfo.cryptoFunctions.randomFxn = testRandomFxn;
    initInfo.cryptoFunctions.crc32Fxn = NULL;

    result = Ice_Init( &( context ),
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_BAD_PARAM,
                       result );

    initInfo.cryptoFunctions.crc32Fxn = testCrc32Fxn;
    initInfo.cryptoFunctions.hmacFxn = NULL;

    result = Ice_Init( &( context ),
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_BAD_PARAM,
                       result );

    initInfo.cryptoFunctions.hmacFxn = testHmacFxn;
    initInfo.creds.pLocalUsername = NULL;

    result = Ice_Init( &( context ),
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_BAD_PARAM,
                       result );

    initInfo.creds.pLocalUsername = &( localUsername[ 0 ] );
    initInfo.creds.pLocalPassword = NULL;

    result = Ice_Init( &( context ),
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_BAD_PARAM,
                       result );

    initInfo.creds.pLocalPassword = &( localPassword[ 0 ] );
    initInfo.creds.pRemoteUsername = NULL;

    result = Ice_Init( &( context ),
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_BAD_PARAM,
                       result );

    initInfo.creds.pRemoteUsername = &( remoteUsername[ 0 ] );
    initInfo.creds.pRemotePassword = NULL;

    result = Ice_Init( &( context ),
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_BAD_PARAM,
                       result );

    initInfo.creds.pRemotePassword = &( remotePassword[ 0 ] );
    initInfo.creds.pCombinedUsername = NULL;

    result = Ice_Init( &( context ),
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate ICE Init functionality.
 */
void test_iceInit( void )
{
    IceContext_t context = { 0 };
    IceResult_t result;

    result = Ice_Init( &( context ),
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_OK,
                       result );

    TEST_ASSERT_EQUAL( initInfo.creds.localUsernameLength,
                       context.creds.localUsernameLength );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( initInfo.creds.pLocalUsername,
                                   context.creds.pLocalUsername,
                                   initInfo.creds.localUsernameLength );

    TEST_ASSERT_EQUAL( initInfo.creds.localPasswordLength,
                       context.creds.localPasswordLength );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( initInfo.creds.pLocalPassword,
                                   context.creds.pLocalPassword,
                                   initInfo.creds.localPasswordLength );

    TEST_ASSERT_EQUAL( initInfo.creds.remoteUsernameLength,
                       context.creds.remoteUsernameLength );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( initInfo.creds.pRemoteUsername,
                                   context.creds.pRemoteUsername,
                                   initInfo.creds.remoteUsernameLength );

    TEST_ASSERT_EQUAL( initInfo.creds.remotePasswordLength,
                       context.creds.remotePasswordLength );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( initInfo.creds.pRemotePassword,
                                   context.creds.pRemotePassword,
                                   initInfo.creds.remotePasswordLength );

    TEST_ASSERT_EQUAL( initInfo.creds.combinedUsernameLength,
                       context.creds.combinedUsernameLength );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( initInfo.creds.pCombinedUsername,
                                   context.creds.pCombinedUsername,
                                   initInfo.creds.combinedUsernameLength );

    TEST_ASSERT_EQUAL( &( localCandidateArray[ 0 ] ),
                       context.pLocalCandidates );
    TEST_ASSERT_EQUAL( 10,
                       context.maxLocalCandidates );
    TEST_ASSERT_EQUAL( 0,
                       context.numLocalCandidates );

    TEST_ASSERT_EQUAL( &( remoteCandidateArray[ 0 ] ),
                       context.pRemoteCandidates );
    TEST_ASSERT_EQUAL( 10,
                       context.maxRemoteCandidates );
    TEST_ASSERT_EQUAL( 0,
                       context.numRemoteCandidates );

    TEST_ASSERT_EQUAL( &( candidatePairArray[ 0 ] ),
                       context.pCandidatePairs );
    TEST_ASSERT_EQUAL( 100,
                       context.maxCandidatePairs );
    TEST_ASSERT_EQUAL( 0,
                       context.numCandidatePairs );

    TEST_ASSERT_EQUAL( 1,
                       context.isControlling );
    TEST_ASSERT_EQUAL( &( transactionIdStore[ 0 ] ),
                       context.pStunBindingRequestTransactionIdStore );
    TEST_ASSERT_EQUAL( testRandomFxn,
                       context.cryptoFunctions.randomFxn );
    TEST_ASSERT_EQUAL( testCrc32Fxn,
                       context.cryptoFunctions.crc32Fxn );
    TEST_ASSERT_EQUAL( testHmacFxn,
                       context.cryptoFunctions.hmacFxn );
    TEST_ASSERT_EQUAL( 0,
                       context.numLocalCandidates );
    TEST_ASSERT_EQUAL( 0,
                       context.maxLocalCandidates );
    TEST_ASSERT_EQUAL( 0,
                       context.numRemoteCandidates );
    TEST_ASSERT_EQUAL( 0,
                       context.maxRemoteCandidates );
    TEST_ASSERT_EQUAL( 0,
                       context.maxCandidatePairs );
    TEST_ASSERT_EQUAL( 0,
                       context.numCandidatePairs );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate ICE Add Host Candidate fail functionality for Bad Parameters.
 */
void test_iceAddHostCandidate_BadParams( void )
{
    IceContext_t context = { 0 };
    IceEndpoint_t endPoint = { 0 };
    IceResult_t result;

    result = Ice_AddHostCandidate( NULL,
                                   &( endPoint ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_BAD_PARAM,
                       result );

    result = Ice_AddHostCandidate( &( context ),
                                   NULL );

    TEST_ASSERT_EQUAL( ICE_RESULT_BAD_PARAM,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate ICE Add Host Candidate fail functionality for Max Candidate Threshold.
 */
void test_iceAddHostCandidate_MaxCandidateThreshold( void )
{
    IceContext_t context = { 0 };
    IceEndpoint_t endPoint = { 0 };
    IceResult_t result;

    result = Ice_Init( &( context ),
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_OK,
                       result );

    context.numLocalCandidates = 1000;
    context.maxLocalCandidates = 1000;

    result = Ice_AddHostCandidate( &( context ),
                                   &( endPoint ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_MAX_CANDIDATE_THRESHOLD,
                       result );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate ICE Add Host Candidate functionality.
 */
void test_iceAddHostCandidate( void )
{
    IceContext_t context = { 0 };
    IceCandidate_t localCandidates[ 5 ];
    IceEndpoint_t endPoint = { 0 };
    IceResult_t result;
    const uint8_t * ipAddress;

    result = Ice_Init( &( context ),
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_OK,
                       result );

    context.pLocalCandidates = &( localCandidates[ 0 ] );
    context.maxLocalCandidates = 5;
    context.numLocalCandidates = 0;

    endPoint.isPointToPoint = 1;
    ipAddress = ( uint8_t * ) "192.168.1.100";
    endPoint.transportAddress.family = 0;
    memcpy( endPoint.transportAddress.address,
            ipAddress,
            16 );
    endPoint.transportAddress.port = 8080;
    memcpy( ( void * ) &( endPoint.transportAddress.address[ 0 ] ),
            ( const void * ) "192.168.1.100",
            strlen( "192.168.1.100" ) );

    result = Ice_AddHostCandidate( &( context ),
                                   &( endPoint ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 1,
                       context.numLocalCandidates );
    TEST_ASSERT_EQUAL( ICE_CANDIDATE_TYPE_HOST,
                       context.pLocalCandidates[ 0 ].candidateType );
    TEST_ASSERT_EQUAL( 0,
                       context.pLocalCandidates[ 0 ].isRemote );
    TEST_ASSERT_EQUAL( 1,
                       context.pLocalCandidates[ 0 ].endpoint.isPointToPoint );
    TEST_ASSERT_EQUAL( 0,
                       context.pLocalCandidates[0].isRemote );
    TEST_ASSERT_EQUAL( HOST_CANDIDATE_PRIORITY,
                       context.pLocalCandidates[0].priority );
    TEST_ASSERT_EQUAL( ICE_SOCKET_PROTOCOL_NONE,
                       context.pLocalCandidates[ 0 ].remoteProtocol );
    TEST_ASSERT_EQUAL( ICE_CANDIDATE_STATE_VALID,
                       context.pLocalCandidates[0].state );
    TEST_ASSERT_EQUAL( 1,
                       context.pLocalCandidates[0].endpoint.isPointToPoint );
    TEST_ASSERT_EQUAL( 0,
                       context.pLocalCandidates[0].endpoint.transportAddress.family );
    TEST_ASSERT_EQUAL( 8080,
                       context.pLocalCandidates[0].endpoint.transportAddress.port );
    TEST_ASSERT_EQUAL_STRING( "192.168.1.100",
                              context.pLocalCandidates[0].endpoint.transportAddress.address );
}

/*-----------------------------------------------------------*/

/**
 * @brief Validate ICE Add Host Candidate functionality for Multicast or Broadcast Connection.
 */
void test_iceAddHostCandidate_Multicast( void )
{
    IceContext_t context = { 0 };
    IceCandidate_t localCandidates[5];
    IceEndpoint_t endPoint = { 0 };
    IceResult_t result;
    const uint8_t * ipAddress;

    result = Ice_Init( &( context ),
                       &( initInfo ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_OK,
                       result );

    context.pLocalCandidates = &( localCandidates[ 0 ] );
    context.maxLocalCandidates = 5;
    context.numLocalCandidates = 0;

    endPoint.isPointToPoint = 0;
    ipAddress = ( uint8_t * ) "192.168.1.100";
    endPoint.transportAddress.family = 0;
    memcpy( endPoint.transportAddress.address,
            ipAddress,
            16 );
    endPoint.transportAddress.port = 8080;

    result = Ice_AddHostCandidate( &( context ),
                                   &( endPoint ) );

    TEST_ASSERT_EQUAL( ICE_RESULT_OK,
                       result );
    TEST_ASSERT_EQUAL( 1,
                       context.numLocalCandidates );
    TEST_ASSERT_EQUAL( ICE_CANDIDATE_TYPE_HOST,
                       context.pLocalCandidates[0].candidateType );
    TEST_ASSERT_EQUAL( 0,
                       context.pLocalCandidates[0].isRemote );
    TEST_ASSERT_EQUAL( HOST_CANDIDATE_PRIORITY_MULTICAST,
                       context.pLocalCandidates[0].priority );
    TEST_ASSERT_EQUAL( ICE_SOCKET_PROTOCOL_NONE,
                       context.pLocalCandidates[0].remoteProtocol );
    TEST_ASSERT_EQUAL( ICE_CANDIDATE_STATE_VALID,
                       context.pLocalCandidates[0].state );
    TEST_ASSERT_EQUAL( 0,
                       context.pLocalCandidates[0].endpoint.isPointToPoint );
    TEST_ASSERT_EQUAL( 0,
                       context.pLocalCandidates[0].endpoint.transportAddress.family );
    TEST_ASSERT_EQUAL( 8080,
                       context.pLocalCandidates[0].endpoint.transportAddress.port );
    TEST_ASSERT_EQUAL_STRING( "192.168.1.100",
                              context.pLocalCandidates[0].endpoint.transportAddress.address );
}

/*-----------------------------------------------------------*/
