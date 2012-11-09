/* -*- Mode:C++; c-file-style:"microsoft"; indent-tabs-mode:nil; -*- */

#include "ns3/config.h"
#include "ns3/integer.h"
#include "ns3/node.h"
#include "ns3/simulator.h"

#include "ns3/tocino-net-device.h"
#include "ns3/tocino-channel.h"
#include "ns3/tocino-helper.h"
#include "ns3/tocino-misc.h"

#include "test-tocino-3d-torus.h"

using namespace ns3;

TestTocino3DTorus::TestTocino3DTorus()
  : TestCase( "Test a 3D Torus" )
{}

TestTocino3DTorus::~TestTocino3DTorus()
{}

int TestTocino3DTorus::Inc( const int i ) const
{
    return ( (i + m_radix) + 1 ) % m_radix;
}

int TestTocino3DTorus::Dec( const int i ) const
{
    return ( (i + m_radix) - 1 ) % m_radix;
}

bool TestTocino3DTorus::AcceptPacket( Ptr<NetDevice> nd, Ptr<const Packet> p, uint16_t, const Address& src )
{
    TocinoAddress tsrc = TocinoAddress::ConvertFrom( src );
    TocinoAddress tdst = TocinoAddress::ConvertFrom( nd->GetAddress() );

    m_counts[tsrc][tdst]++;
    m_bytes[tsrc][tdst] += p->GetSize();

    return true;
}

void TestTocino3DTorus::Initialize()
{
    NS_ASSERT( m_radix >= 0 );

    // create net devices
    m_netDevices.resize(m_radix);
    for( int x = 0; x < m_radix; x++ )
    { 
        m_netDevices[x].resize(m_radix);
        for( int y = 0; y < m_radix; y++ )
        { 
            m_netDevices[x][y].resize(m_radix);
            for( int z = 0; z < m_radix; z++ )
            {
                Ptr<TocinoNetDevice> tnd = CreateObject<TocinoNetDevice>();
                
                tnd->Initialize();
                tnd->SetAddress( TocinoAddress( x, y, z ) );
                tnd->SetReceiveCallback( MakeCallback( &TestTocino3DTorus::AcceptPacket, this ) );

                // HACK: The Nodes are required to avoid
                // SIGSEGV in TocinoChannel::TransmitEnd()
                tnd->SetNode( CreateObject<Node>() );

                m_netDevices[x][y][z] = tnd;
            }
        }
    }
 
    // create channels and interconnect net devices
    for( int x = 0; x < m_radix; x++ )
    { 
        for( int y = 0; y < m_radix; y++ )
        { 
            for( int z = 0; z < m_radix; z++ )
            {
                Ptr<TocinoNetDevice> cur = m_netDevices[x][y][z];

                TocinoChannelHelper( cur, 0, m_netDevices[ Inc(x) ][y][z], 1 ); // x+
                TocinoChannelHelper( cur, 1, m_netDevices[ Dec(x) ][y][z], 0 ); // x-

                TocinoChannelHelper( cur, 2, m_netDevices[x][ Inc(y) ][z], 3 ); // y+
                TocinoChannelHelper( cur, 3, m_netDevices[x][ Dec(y) ][z], 2 ); // y-

                TocinoChannelHelper( cur, 4, m_netDevices[x][y][ Inc(z) ], 5 ); // z+
                TocinoChannelHelper( cur, 5, m_netDevices[x][y][ Dec(z) ], 4 ); // z-
            }
        }
    }
}

void TestTocino3DTorus::Reset()
{
    m_counts.clear();
    m_bytes.clear();
}

unsigned TestTocino3DTorus::GetTotalCount() const
{
    unsigned total = 0;

    TestMatrix::const_iterator i;
    TestMatrixRow::const_iterator j;

    for( i = m_counts.begin(); i != m_counts.end(); i++ ) 
    {
        const TestMatrixRow& row = m_counts.find(i->first)->second;

        for( j = row.begin(); j != row.end(); j++ ) 
        {
            total += j->second;
        }
    }

    return total;
}

unsigned TestTocino3DTorus::GetTotalBytes()
{
    unsigned total = 0;
    
    TestMatrix::iterator i;
    TestMatrixRow::iterator j;
    
    for( i = m_bytes.begin(); i != m_bytes.end(); i++ ) 
    {
        const TestMatrixRow& row = m_bytes.find(i->first)->second;

        for( j = row.begin(); j != row.end(); j++ ) 
        {
            total += j->second;
        }
    }

    return total;
}

void TestTocino3DTorus::RunOneTest( const unsigned COUNT, const unsigned BYTES, const TocinoAddress& src, const TocinoAddress& dst )
{
    TocinoCustomizeLogging();

    Ptr<Packet> p = Create<Packet>( BYTES );
   
    Ptr<TocinoNetDevice> srcNetDevice = m_netDevices[src.GetX()][src.GetY()][src.GetZ()];

    Reset();

    for( unsigned i = 0; i < COUNT; ++i )
    {
        Simulator::ScheduleWithContext( srcNetDevice->GetNode()->GetId(), Seconds(0),
                &TocinoNetDevice::Send, srcNetDevice, p, dst, 0 );
    }

    Simulator::Run();
   
    NS_TEST_ASSERT_MSG_EQ( m_counts[src][dst], COUNT, "Unexpected packet count" );
    NS_TEST_ASSERT_MSG_EQ( m_bytes[src][dst], BYTES*COUNT, "Unexpected packet bytes" );
    
    NS_TEST_ASSERT_MSG_EQ( GetTotalCount(), COUNT, "Unexpected total packet count" );
    NS_TEST_ASSERT_MSG_EQ( GetTotalBytes(), BYTES*COUNT, "Unexpected total packet bytes" );
    
    Simulator::Destroy();
}

TocinoAddress TestTocino3DTorus::OppositeCorner( const TocinoAddress& a )
{
    int x = a.GetX();
    int y = a.GetY();
    int z = a.GetZ();

    if( x == 0 ) { x = m_radix-1; }
    else { NS_ASSERT( x == m_radix-1 ); }
    
    if( y == 0 ) { y = m_radix-1; }
    else { NS_ASSERT( y == m_radix-1 ); }
    
    if( z == 0 ) { z = m_radix-1; }
    else { NS_ASSERT( z == m_radix-1 ); }

    return TocinoAddress( x, y, z );
}

void TestTocino3DTorus::TestHelper( const unsigned COUNT, const unsigned BYTES )
{
    // iterate over the "corners"
    for( int x = 0; x < m_radix; x += (m_radix-1) )
    { 
        for( int y = 0; y < m_radix; y += (m_radix-1) )
        { 
            for( int z = 0; z < m_radix; z += (m_radix-1) )
            {
                TocinoAddress src( x, y, z );
                TocinoAddress dst = OppositeCorner( src );

                RunOneTest( COUNT, BYTES, src, dst );
            }
        }
    }
}

void
TestTocino3DTorus::DoRun()
{
    m_radix = 3;

    Initialize();

    TestHelper( 1, 20 );
    TestHelper( 1, 123 );
    TestHelper( 2, 32 );

    Config::SetDefault( "ns3::TocinoDimensionOrderRouter::WrapPoint", IntegerValue( m_radix-1 ) );
    
    TestHelper( 1, 20 );
    TestHelper( 1, 123 );
    TestHelper( 2, 32 );

    Config::Reset();
}