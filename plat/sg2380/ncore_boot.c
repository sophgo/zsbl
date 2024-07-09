#include <lib/mmio.h>

void ncore_direct_config()
{
	//P670_0_ace_caiu		base address:0x508300_0000
	mmio_write_32(0x5083000000,0x80000000); //id
	mmio_write_32(0x5083000004,0x80000000); //fab id
	mmio_write_32(0x5083000040,0x000f0200); //coattach
	mmio_write_32(0x5083000100,0x0000003f); //uncorrectable error detect
	mmio_write_32(0x5083000104,0x0000003f); //uncorrectable error interrupt
	mmio_write_32(0x5083000108,0x00000001); //uncorrectable error state enable
	mmio_write_32(0x5083000140,0x00000803); //correctable error control 
	mmio_write_32(0x5083000144,0x00000001); //correctable error status
	mmio_write_32(0x5083000150,0x000003ff); //correctable error status alias
	mmio_write_32(0x508300017c,0x00000001); //correctable error resiliency threshold
	mmio_write_32(0x5083000190,0x00004000); //timeout threshold
	mmio_write_32(0x5083000194,0x00004000); //event timeout threshold
	mmio_write_32(0x5083000198,0x00004000); //co-protocol timeout threshold
	mmio_write_32(0x508300019c,0x00004000); //event handshake timeout threshold
	mmio_write_32(0x5083000200,0x001e0010); //QOS
	mmio_write_32(0x5083000380,0x00050830); //NRS
	mmio_write_32(0x50830003a0,0x00000000); //Boot Region Attribute Register
	mmio_write_32(0x50830003a4,0x00000000); //low addr
	mmio_write_32(0x50830003a8,0x00000000); //high addr
	mmio_write_32(0x50830003c0,0x00000001); //Active Memory Interleave Group Register 
	mmio_write_32(0x50830003c4,0x00000000); //Memory Interleave Function Select Register 
	mmio_write_32(0x5083000400,0x81600000); //General Purpose Region Attribute Register
	mmio_write_32(0x5083000404,0x00080000); //General Purpose Region addr low 43:12
	mmio_write_32(0x5083000408,0x00000000); //General Purpose Region addr high 51:44
	mmio_write_32(0x5083000900,0x10020000);
	mmio_write_32(0x5083000b0c,0x40000000);
	mmio_write_32(0x5083000b1c,0x40000000);
	mmio_write_32(0x5083000b2c,0x40000000);
	mmio_write_32(0x5083000b3c,0x40000000);
	mmio_write_32(0x5083000c00,0x00000804); //Credit
	mmio_write_32(0x5083000c04,0x00000804); //Credit
	mmio_write_32(0x5083000c08,0x00000804); //Credit
	mmio_write_32(0x5083000c0c,0x00000804); //Credit
	mmio_write_32(0x5083000c10,0x00000804); //Credit
	mmio_write_32(0x5083000c14,0x00000804); //Credit
	mmio_write_32(0x5083000c18,0x00000804); //Credit
	mmio_write_32(0x5083000c1c,0x00000804); //Credit
	//P670_1_ace_caiu		base address:0x508300_1000
	mmio_write_32(0x5083001000,0x80001001); //id
	mmio_write_32(0x5083001004,0x80000001); //fab id
	mmio_write_32(0x5083001040,0x000f0200); //coattach
	mmio_write_32(0x5083001100,0x0000003f); //uncorrectable error detect
	mmio_write_32(0x5083001104,0x0000003f); //uncorrectable error interrupt
	mmio_write_32(0x5083001108,0x00000001); //uncorrectable error state enable
	mmio_write_32(0x5083001140,0x00000803); //correctable error control 
	mmio_write_32(0x5083001144,0x00000001); //correctable error status
	mmio_write_32(0x5083001150,0x000003ff); //correctable error status alias
	mmio_write_32(0x508300117c,0x00000001); //correctable error resiliency threshold
	mmio_write_32(0x5083001190,0x00004000); //timeout threshold
	mmio_write_32(0x5083001194,0x00004000); //event timeout threshold
	mmio_write_32(0x5083001198,0x00004000); //co-protocol timeout threshold
	mmio_write_32(0x508300119c,0x00004000); //event handshake timeout threshold
	mmio_write_32(0x5083001200,0x001e0010); //QOS
	mmio_write_32(0x5083001380,0x00050830); //NRS
	mmio_write_32(0x50830013a0,0x00000000); //Boot Region Attribute Register
	mmio_write_32(0x50830013a4,0x00000000); //low addr
	mmio_write_32(0x50830013a8,0x00000000); //high addr
	mmio_write_32(0x50830013c0,0x00000001); //Active Memory Interleave Group Register
	mmio_write_32(0x50830013c4,0x00000000); //Memory Interleave Function Select Register
	mmio_write_32(0x5083001400,0x81600000); //General Purpose Region Attribute Register
	mmio_write_32(0x5083001404,0x00080000); //General Purpose Region addr low 43:12
	mmio_write_32(0x5083001408,0x00000000); //General Purpose Region addr high 51:44
	mmio_write_32(0x5083001900,0x10020000);
	mmio_write_32(0x5083001b0c,0x40000000);
	mmio_write_32(0x5083001b1c,0x40000000);
	mmio_write_32(0x5083001b2c,0x40000000);
	mmio_write_32(0x5083001b3c,0x40000000);
	mmio_write_32(0x5083001c00,0x00000804); //Credit
	mmio_write_32(0x5083001c04,0x00000804); //Credit
	mmio_write_32(0x5083001c08,0x00000804); //Credit
	mmio_write_32(0x5083001c0c,0x00000804); //Credit
	mmio_write_32(0x5083001c10,0x00000804); //Credit
	mmio_write_32(0x5083001c14,0x00000804); //Credit
	mmio_write_32(0x5083001c18,0x00000804); //Credit
	mmio_write_32(0x5083001c1c,0x00000804); //Credit
	//P670_2_ace_caiu		base address:0x508300_2000
	mmio_write_32(0x5083002000,0x80002002); //id
	mmio_write_32(0x5083002004,0x80000002); //fab id
	mmio_write_32(0x5083002040,0x000f0200); //coattach
	mmio_write_32(0x5083002100,0x0000003f); //uncorrectable error detect
	mmio_write_32(0x5083002104,0x0000003f); //uncorrectable error interrupt
	mmio_write_32(0x5083002108,0x00000001); //uncorrectable error state enable
	mmio_write_32(0x5083002140,0x00000803); //correctable error control 
	mmio_write_32(0x5083002144,0x00000001); //correctable error status
	mmio_write_32(0x5083002150,0x000003ff); //correctable error status alias
	mmio_write_32(0x508300217c,0x00000001); //correctable error resiliency threshold
	mmio_write_32(0x5083002190,0x00004000); //timeout threshold
	mmio_write_32(0x5083002194,0x00004000); //event timeout threshold
	mmio_write_32(0x5083002198,0x00004000); //co-protocol timeout threshold
	mmio_write_32(0x508300219c,0x00004000); //event handshake timeout threshold
	mmio_write_32(0x5083002200,0x001e0010); //QOS
	mmio_write_32(0x5083002380,0x00050830); //NRS
	mmio_write_32(0x50830023a0,0x00000000); //Boot Region Attribute Register
	mmio_write_32(0x50830023a4,0x00000000); //low addr
	mmio_write_32(0x50830023a8,0x00000000); //high addr
	mmio_write_32(0x50830023c0,0x00000001); //Active Memory Interleave Group Register
	mmio_write_32(0x50830023c4,0x00000000); //Memory Interleave Function Select Register
	mmio_write_32(0x5083002400,0x81600000); //General Purpose Region Attribute Register
	mmio_write_32(0x5083002404,0x00080000); //General Purpose Region addr low 43:12
	mmio_write_32(0x5083002408,0x00000000); //General Purpose Region addr high 51:44
	mmio_write_32(0x5083002900,0x10020000);
	mmio_write_32(0x5083002b0c,0x40000000);
	mmio_write_32(0x5083002b1c,0x40000000);
	mmio_write_32(0x5083002b2c,0x40000000);
	mmio_write_32(0x5083002b3c,0x40000000);
	mmio_write_32(0x5083002c00,0x00000804); //Credit
	mmio_write_32(0x5083002c04,0x00000804); //Credit
	mmio_write_32(0x5083002c08,0x00000804); //Credit
	mmio_write_32(0x5083002c0c,0x00000804); //Credit
	mmio_write_32(0x5083002c10,0x00000804); //Credit
	mmio_write_32(0x5083002c14,0x00000804); //Credit
	mmio_write_32(0x5083002c18,0x00000804); //Credit
	mmio_write_32(0x5083002c1c,0x00000804); //Credit
	//P670_3_ace_caiu		base address:0x508300_3000
	mmio_write_32(0x5083003000,0x80003003); //id
	mmio_write_32(0x5083003004,0x80000003); //fab id
	mmio_write_32(0x5083003040,0x000f0200); //coattach
	mmio_write_32(0x5083003100,0x0000003f); //uncorrectable error detect
	mmio_write_32(0x5083003104,0x0000003f); //uncorrectable error interrupt
	mmio_write_32(0x5083003108,0x00000001); //uncorrectable error state enable
	mmio_write_32(0x5083003140,0x00000803); //correctable error control 
	mmio_write_32(0x5083003144,0x00000001); //correctable error status
	mmio_write_32(0x5083003150,0x000003ff); //correctable error status alias
	mmio_write_32(0x508300317c,0x00000001); //correctable error resiliency threshold
	mmio_write_32(0x5083003190,0x00004000); //timeout threshold
	mmio_write_32(0x5083003194,0x00004000); //event timeout threshold
	mmio_write_32(0x5083003198,0x00004000); //co-protocol timeout threshold
	mmio_write_32(0x508300319c,0x00004000); //event handshake timeout threshold
	mmio_write_32(0x5083003200,0x001e0010); //QOS
	mmio_write_32(0x5083003380,0x00050830); //NRS
	mmio_write_32(0x50830033a0,0x00000000); //Boot Region Attribute Register
	mmio_write_32(0x50830033a4,0x00000000); //low addr
	mmio_write_32(0x50830033a8,0x00000000); //high addr
	mmio_write_32(0x50830033c0,0x00000001); //Active Memory Interleave Group Register
	mmio_write_32(0x50830033c4,0x00000000); //Memory Interleave Function Select Register
	mmio_write_32(0x5083003400,0x81600000); //General Purpose Region Attribute Register
	mmio_write_32(0x5083003404,0x00080000); //General Purpose Region addr low 43:12
	mmio_write_32(0x5083003408,0x00000000); //General Purpose Region addr high 51:44
	mmio_write_32(0x5083003900,0x10020000);
	mmio_write_32(0x5083003b0c,0x40000000);
	mmio_write_32(0x5083003b1c,0x40000000);
	mmio_write_32(0x5083003b2c,0x40000000);
	mmio_write_32(0x5083003b3c,0x40000000);
	mmio_write_32(0x5083003c00,0x00000804); //Credit
	mmio_write_32(0x5083003c04,0x00000804); //Credit
	mmio_write_32(0x5083003c08,0x00000804); //Credit
	mmio_write_32(0x5083003c0c,0x00000804); //Credit
	mmio_write_32(0x5083003c10,0x00000804); //Credit
	mmio_write_32(0x5083003c14,0x00000804); //Credit
	mmio_write_32(0x5083003c18,0x00000804); //Credit
	mmio_write_32(0x5083003c1c,0x00000804); //Credit
	//GPU_0_acelite_io_aiu		base address:0x508300_4000
	mmio_write_32(0x5083004000,0x80004004); //id
	mmio_write_32(0x5083004004,0x80000004); //fab id
	mmio_write_32(0x5083004040,0x000a0000); //coattach
	mmio_write_32(0x5083004100,0x0000003f); //uncorrectable error detect
	mmio_write_32(0x5083004104,0x0000003f); //uncorrectable error interrupt
	mmio_write_32(0x5083004108,0x00000001); //uncorrectable error state enable
	mmio_write_32(0x5083004140,0x00000803); //correctable error control 
	mmio_write_32(0x5083004144,0x00000001); //correctable error status
	mmio_write_32(0x5083004150,0x000003ff); //correctable error status alias
	mmio_write_32(0x508300417c,0x00000001); //correctable error resiliency threshold
	mmio_write_32(0x5083004190,0x00004000); //timeout threshold
	mmio_write_32(0x5083004194,0x00004000); //event timeout threshold
	mmio_write_32(0x5083004198,0x00004000); //co-protocol timeout threshold
	mmio_write_32(0x508300419c,0x00004000); //event handshake timeout threshold
	mmio_write_32(0x5083004200,0x001e0010); //QOS
	mmio_write_32(0x5083004380,0x00050830); //NRS
	mmio_write_32(0x50830043a0,0x00000000); //Boot Region Attribute Register
	mmio_write_32(0x50830043a4,0x00000000); //low addr
	mmio_write_32(0x50830043a8,0x00000000); //high addr
	mmio_write_32(0x50830043c0,0x00000001); //Active Memory Interleave Group Register
	mmio_write_32(0x50830043c4,0x00000000); //Memory Interleave Function Select Register
	mmio_write_32(0x5083004400,0x81600016); //General Purpose Region Attribute Register
	mmio_write_32(0x5083004404,0x00080000); //General Purpose Region addr low 43:12
	mmio_write_32(0x5083004408,0x00000000); //General Purpose Region addr high 51:44
	mmio_write_32(0x5083004900,0x10020000);
	mmio_write_32(0x5083004b0c,0x40000000);
	mmio_write_32(0x5083004b1c,0x40000000);
	mmio_write_32(0x5083004b2c,0x40000000);
	mmio_write_32(0x5083004b3c,0x40000000);
	mmio_write_32(0x5083004c00,0x00000a05); //Credit
	mmio_write_32(0x5083004c04,0x00000a05); //Credit
	mmio_write_32(0x5083004c08,0x00000a05); //Credit
	mmio_write_32(0x5083004c0c,0x00000a05); //Credit
	mmio_write_32(0x5083004c10,0x00000a05); //Credit
	mmio_write_32(0x5083004c14,0x00000a05); //Credit
	mmio_write_32(0x5083004c18,0x00000a05); //Credit
	mmio_write_32(0x5083004c1c,0x00000a05); //Credit
	//PCIe_0_acelite_io_aiu			base address:0x50_8300_5000
	mmio_write_32(0x5083005000,0x80005005); //id
	mmio_write_32(0x5083005004,0x80000005); //fab id
	mmio_write_32(0x5083005040,0x000a0000); //coattach
	mmio_write_32(0x5083005100,0x0000003f); //uncorrectable error detect
	mmio_write_32(0x5083005104,0x0000003f); //uncorrectable error interrupt
	mmio_write_32(0x5083005108,0x00000001); //uncorrectable error state enable
	mmio_write_32(0x5083005140,0x00000803); //correctable error control 
	mmio_write_32(0x5083005144,0x00000001); //correctable error status
	mmio_write_32(0x5083005150,0x000003ff); //correctable error status alias
	mmio_write_32(0x508300517c,0x00000001); //correctable error resiliency threshold
	mmio_write_32(0x5083005190,0x00004000); //timeout threshold
	mmio_write_32(0x5083005194,0x00004000); //event timeout threshold
	mmio_write_32(0x5083005198,0x00004000); //co-protocol timeout threshold
	mmio_write_32(0x508300519c,0x00004000); //event handshake timeout threshold
	mmio_write_32(0x5083005200,0x001e0010); //QOS
	mmio_write_32(0x5083005380,0x00050830); //NRS
	mmio_write_32(0x50830053a0,0x00000000); //Boot Region Attribute Register
	mmio_write_32(0x50830053a4,0x00000000); //low addr
	mmio_write_32(0x50830053a8,0x00000000); //high addr
	mmio_write_32(0x50830053c0,0x00000001); //Active Memory Interleave Group Register
	mmio_write_32(0x50830053c4,0x00000000); //Memory Interleave Function Select Register
	mmio_write_32(0x5083005400,0x81600016); //General Purpose Region Attribute Register
	mmio_write_32(0x5083005404,0x00080000); //General Purpose Region addr low 43:12
	mmio_write_32(0x5083005408,0x00000000); //General Purpose Region addr high 51:44
	mmio_write_32(0x5083005900,0x10020000);
	mmio_write_32(0x5083005b0c,0x40000000);
	mmio_write_32(0x5083005b1c,0x40000000);
	mmio_write_32(0x5083005b2c,0x40000000);
	mmio_write_32(0x5083005b3c,0x40000000);
	mmio_write_32(0x5083005c00,0x00000a05); //Credit
	mmio_write_32(0x5083005c04,0x00000a05); //Credit
	mmio_write_32(0x5083005c08,0x00000a05); //Credit
	mmio_write_32(0x5083005c0c,0x00000a05); //Credit
	mmio_write_32(0x5083005c10,0x00000a05); //Credit
	mmio_write_32(0x5083005c14,0x00000a05); //Credit
	mmio_write_32(0x5083005c18,0x00000a05); //Credit
	mmio_write_32(0x5083005c1c,0x00000a05); //Credit
	//USB_PERI_0_acelite_io_aiu		base address:0x508300_6000
	mmio_write_32(0x5083006000,0x80006006); //id
	mmio_write_32(0x5083006004,0x80000006); //fab id
	mmio_write_32(0x5083006040,0x000a0000); //coattach
	mmio_write_32(0x5083006100,0x0000003f); //uncorrectable error detect
	mmio_write_32(0x5083006104,0x0000003f); //uncorrectable error interrupt
	mmio_write_32(0x5083006108,0x00000001); //uncorrectable error state enable
	mmio_write_32(0x5083006140,0x00000803); //correctable error control 
	mmio_write_32(0x5083006144,0x00000001); //correctable error status
	mmio_write_32(0x5083006150,0x000003ff); //correctable error status alias
	mmio_write_32(0x508300617c,0x00000001); //correctable error resiliency threshold
	mmio_write_32(0x5083006190,0x00004000); //timeout threshold
	mmio_write_32(0x5083006194,0x00004000); //event timeout threshold
	mmio_write_32(0x5083006198,0x00004000); //co-protocol timeout threshold
	mmio_write_32(0x508300619c,0x00004000); //event handshake timeout threshold
	mmio_write_32(0x5083006200,0x001e0010); //QOS
	mmio_write_32(0x5083006380,0x00050830); //NRS
	mmio_write_32(0x50830063a0,0x00000000); //Boot Region Attribute Register
	mmio_write_32(0x50830063a4,0x00000000); //low addr
	mmio_write_32(0x50830063a8,0x00000000); //high addr
	mmio_write_32(0x50830063c0,0x00000001); //Active Memory Interleave Group Register
	mmio_write_32(0x50830063c4,0x00000000); //Memory Interleave Function Select Register
	mmio_write_32(0x5083006400,0x81600016); //General Purpose Region Attribute Register
	mmio_write_32(0x5083006404,0x00080000); //General Purpose Region addr low 43:12
	mmio_write_32(0x5083006408,0x00000000); //General Purpose Region addr high 51:44
	mmio_write_32(0x5083006900,0x10020000);
	mmio_write_32(0x5083006b0c,0x40000000);
	mmio_write_32(0x5083006b1c,0x40000000);
	mmio_write_32(0x5083006b2c,0x40000000);
	mmio_write_32(0x5083006b3c,0x40000000);
	mmio_write_32(0x5083006c00,0x00000a05); //Credit
	mmio_write_32(0x5083006c04,0x00000a05); //Credit
	mmio_write_32(0x5083006c08,0x00000a05); //Credit
	mmio_write_32(0x5083006c0c,0x00000a05); //Credit
	mmio_write_32(0x5083006c10,0x00000a05); //Credit
	mmio_write_32(0x5083006c14,0x00000a05); //Credit
	mmio_write_32(0x5083006c18,0x00000a05); //Credit
	mmio_write_32(0x5083006c1c,0x00000a05); //Credit
	//dce0				base address:0x508300_7000
	mmio_write_32(0x5083007000,0x80000007); //id
	mmio_write_32(0x5083007004,0x80000007); //fab id
	mmio_write_32(0x5083007100,0x00000803);
	mmio_write_32(0x5083007104,0x00000001);
	mmio_write_32(0x5083007110,0x000003ff);
	mmio_write_32(0x5083007140,0x0000003f);
	mmio_write_32(0x5083007144,0x0000003f);
	mmio_write_32(0x5083007148,0x00000001);
	mmio_write_32(0x5083007190,0x00004000);
	mmio_write_32(0x5083007194,0x00004000);
	mmio_write_32(0x5083007200,0x001e0010); //QOS
	mmio_write_32(0x50830073a0,0x00000000);
	mmio_write_32(0x50830073a4,0x00000000);
	mmio_write_32(0x50830073a8,0x00000000);
	mmio_write_32(0x50830073c0,0x00000001); //Active Memory Interleave Group Register
	mmio_write_32(0x50830073c4,0x00000000); //Memory Interleave Function Select Register
	mmio_write_32(0x5083007400,0x81600000); //General Purpose Region Attribute Register
	mmio_write_32(0x5083007404,0x00080000); //General Purpose Region addr low 43:12
	mmio_write_32(0x5083007408,0x00000000); //General Purpose Region addr high 51:44
	mmio_write_32(0x5083007c00,0x0000001f); //Credit
	mmio_write_32(0x5083007c04,0x00000000); //Credit
	mmio_write_32(0x5083007c08,0x00000000); //Credit
	mmio_write_32(0x5083007c0c,0x00000000); //Credit
	mmio_write_32(0x5083007c10,0x00000000); //Credit
	mmio_write_32(0x5083007c14,0x00000000); //Credit
	mmio_write_32(0x5083007c18,0x00000000); //Credit
	mmio_write_32(0x5083007c1c,0x00000000); //Credit
	//dce1				base address:0x508300_8000
	mmio_write_32(0x5083008000,0x80001008); //id
	mmio_write_32(0x5083008004,0x80000008); //fab id
	mmio_write_32(0x5083008100,0x00000803);
	mmio_write_32(0x5083008104,0x00000001);
	mmio_write_32(0x5083008110,0x000003ff);
	mmio_write_32(0x5083008140,0x0000003f);
	mmio_write_32(0x5083008144,0x0000003f);
	mmio_write_32(0x5083008148,0x00000001);
	mmio_write_32(0x5083008190,0x00004000);
	mmio_write_32(0x5083008194,0x00004000);
	mmio_write_32(0x5083008200,0x001e0010); //QOS
	mmio_write_32(0x50830083a0,0x00000000);
	mmio_write_32(0x50830083a4,0x00000000);
	mmio_write_32(0x50830083a8,0x00000000);
	mmio_write_32(0x50830083c0,0x00000001); //Active Memory Interleave Group Register
	mmio_write_32(0x50830083c4,0x00000000); //Memory Interleave Function Select Register
	mmio_write_32(0x5083008400,0x81600000); //General Purpose Region Attribute Register
	mmio_write_32(0x5083008404,0x00080000); //General Purpose Region addr low 43:12
	mmio_write_32(0x5083008408,0x00000000); //General Purpose Region addr high 51:44
	mmio_write_32(0x5083008c00,0x00000000); //Credit
	mmio_write_32(0x5083008c04,0x0000001f); //Credit
	mmio_write_32(0x5083008c08,0x00000000); //Credit
	mmio_write_32(0x5083008c0c,0x00000000); //Credit
	mmio_write_32(0x5083008c10,0x00000000); //Credit
	mmio_write_32(0x5083008c14,0x00000000); //Credit
	mmio_write_32(0x5083008c18,0x00000000); //Credit
	mmio_write_32(0x5083008c1c,0x00000000); //Credit
	//dce2				base address:0x508300_9000
	mmio_write_32(0x5083009000,0x80002009); //id
	mmio_write_32(0x5083009004,0x80000009); //fab id
	mmio_write_32(0x5083009100,0x00000803);
	mmio_write_32(0x5083009104,0x00000001);
	mmio_write_32(0x5083009110,0x000003ff);
	mmio_write_32(0x5083009140,0x0000003f);
	mmio_write_32(0x5083009144,0x0000003f);
	mmio_write_32(0x5083009148,0x00000001);
	mmio_write_32(0x5083009190,0x00004000);
	mmio_write_32(0x5083009194,0x00004000);
	mmio_write_32(0x5083009200,0x001e0010); //QOS
	mmio_write_32(0x50830093a0,0x00000000);
	mmio_write_32(0x50830093a4,0x00000000);
	mmio_write_32(0x50830093a8,0x00000000);
	mmio_write_32(0x50830093c0,0x00000001); //Active Memory Interleave Group Register
	mmio_write_32(0x50830093c4,0x00000000); //Memory Interleave Function Select Register
	mmio_write_32(0x5083009400,0x81600000); //General Purpose Region Attribute Register
	mmio_write_32(0x5083009404,0x00080000); //General Purpose Region addr low 43:12
	mmio_write_32(0x5083009408,0x00000000); //General Purpose Region addr high 51:44
	mmio_write_32(0x5083009c00,0x00000000); //Credit
	mmio_write_32(0x5083009c04,0x00000000); //Credit
	mmio_write_32(0x5083009c08,0x0000001f); //Credit
	mmio_write_32(0x5083009c0c,0x00000000); //Credit
	mmio_write_32(0x5083009c10,0x00000000); //Credit
	mmio_write_32(0x5083009c14,0x00000000); //Credit
	mmio_write_32(0x5083009c18,0x00000000); //Credit
	mmio_write_32(0x5083009c1c,0x00000000); //Credit
	//dce3				base addres;s:0x508300_a000
	mmio_write_32(0x508300a000,0x8000300a); //id
	mmio_write_32(0x508300a004,0x8000000a); //fab id
	mmio_write_32(0x508300a100,0x00000803);
	mmio_write_32(0x508300a104,0x00000001);
	mmio_write_32(0x508300a110,0x000003ff);
	mmio_write_32(0x508300a140,0x0000003f);
	mmio_write_32(0x508300a144,0x0000003f);
	mmio_write_32(0x508300a148,0x00000001);
	mmio_write_32(0x508300a190,0x00004000);
	mmio_write_32(0x508300a194,0x00004000);
	mmio_write_32(0x508300a200,0x001e0010); //QOS
	mmio_write_32(0x508300a3a0,0x00000000);
	mmio_write_32(0x508300a3a4,0x00000000);
	mmio_write_32(0x508300a3a8,0x00000000);
	mmio_write_32(0x508300a3c0,0x00000001); //Active Memory Interleave Group Register
	mmio_write_32(0x508300a3c4,0x00000000); //Memory Interleave Function Select Register
	mmio_write_32(0x508300a400,0x81600000); //General Purpose Region Attribute Register
	mmio_write_32(0x508300a404,0x00080000); //General Purpose Region addr low 43:12
	mmio_write_32(0x508300a408,0x00000000); //General Purpose Region addr high 51:44
	mmio_write_32(0x508300ac00,0x00000000); //Credit
	mmio_write_32(0x508300ac04,0x00000000); //Credit
	mmio_write_32(0x508300ac08,0x00000000); //Credit
	mmio_write_32(0x508300ac0c,0x0000001f); //Credit
	mmio_write_32(0x508300ac10,0x00000000); //Credit
	mmio_write_32(0x508300ac14,0x00000000); //Credit
	mmio_write_32(0x508300ac18,0x00000000); //Credit
	mmio_write_32(0x508300ac1c,0x00000000); //Credit
	//dce4				base address:0x508300_b000
	mmio_write_32(0x508300b000,0x8000400b); //id
	mmio_write_32(0x508300b004,0x8000000b); //fab id
	mmio_write_32(0x508300b100,0x00000803);
	mmio_write_32(0x508300b104,0x00000001);
	mmio_write_32(0x508300b110,0x000003ff);
	mmio_write_32(0x508300b140,0x0000003f);
	mmio_write_32(0x508300b144,0x0000003f);
	mmio_write_32(0x508300b148,0x00000001);
	mmio_write_32(0x508300b190,0x00004000);
	mmio_write_32(0x508300b194,0x00004000);
	mmio_write_32(0x508300b200,0x001e0010); //QOS
	mmio_write_32(0x508300b3a0,0x00000000);
	mmio_write_32(0x508300b3a4,0x00000000);
	mmio_write_32(0x508300b3a8,0x00000000);
	mmio_write_32(0x508300b3c0,0x00000001); //Active Memory Interleave Group Register
	mmio_write_32(0x508300b3c4,0x00000000); //Memory Interleave Function Select Register
	mmio_write_32(0x508300b400,0x81600000); //General Purpose Region Attribute Register
	mmio_write_32(0x508300b404,0x00080000); //General Purpose Region addr low 43:12
	mmio_write_32(0x508300b408,0x00000000); //General Purpose Region addr high 51:44
	mmio_write_32(0x508300bc00,0x00000000); //Credit
	mmio_write_32(0x508300bc04,0x00000000); //Credit
	mmio_write_32(0x508300bc08,0x00000000); //Credit
	mmio_write_32(0x508300bc0c,0x00000000); //Credit
	mmio_write_32(0x508300bc10,0x0000001f); //Credit
	mmio_write_32(0x508300bc14,0x00000000); //Credit
	mmio_write_32(0x508300bc18,0x00000000); //Credit
	mmio_write_32(0x508300bc1c,0x00000000); //Credit
	//dce5				base address:0x508300_c000
	mmio_write_32(0x508300c000,0x8000500c); //id
	mmio_write_32(0x508300c004,0x8000000c); //fab id
	mmio_write_32(0x508300c100,0x00000803);
	mmio_write_32(0x508300c104,0x00000001);
	mmio_write_32(0x508300c110,0x000003ff);
	mmio_write_32(0x508300c140,0x0000003f);
	mmio_write_32(0x508300c144,0x0000003f);
	mmio_write_32(0x508300c148,0x00000001);
	mmio_write_32(0x508300c190,0x00004000);
	mmio_write_32(0x508300c194,0x00004000);
	mmio_write_32(0x508300c200,0x001e0010); //QOS
	mmio_write_32(0x508300c3a0,0x00000000);
	mmio_write_32(0x508300c3a4,0x00000000);
	mmio_write_32(0x508300c3a8,0x00000000);
	mmio_write_32(0x508300c3c0,0x00000001); //Active Memory Interleave Group Register
	mmio_write_32(0x508300c3c4,0x00000000); //Memory Interleave Function Select Register
	mmio_write_32(0x508300c400,0x81600000); //General Purpose Region Attribute Register
	mmio_write_32(0x508300c404,0x00080000); //General Purpose Region addr low 43:12
	mmio_write_32(0x508300c408,0x00000000); //General Purpose Region addr high 51:44
	mmio_write_32(0x508300cc00,0x00000000); //Credit
	mmio_write_32(0x508300cc04,0x00000000); //Credit
	mmio_write_32(0x508300cc08,0x00000000); //Credit
	mmio_write_32(0x508300cc0c,0x00000000); //Credit
	mmio_write_32(0x508300cc10,0x00000000); //Credit
	mmio_write_32(0x508300cc14,0x0000001f); //Credit
	mmio_write_32(0x508300cc18,0x00000000); //Credit
	mmio_write_32(0x508300cc1c,0x00000000); //Credit
	//dce6				base address:0x508300_d000
	mmio_write_32(0x508300d000,0x8000600d); //id
	mmio_write_32(0x508300d004,0x8000000d); //fab id
	mmio_write_32(0x508300d100,0x00000803);
	mmio_write_32(0x508300d104,0x00000001);
	mmio_write_32(0x508300d110,0x000003ff);
	mmio_write_32(0x508300d140,0x0000003f);
	mmio_write_32(0x508300d144,0x0000003f);
	mmio_write_32(0x508300d148,0x00000001);
	mmio_write_32(0x508300d190,0x00004000);
	mmio_write_32(0x508300d194,0x00004000);
	mmio_write_32(0x508300d200,0x001e0010); //QOS
	mmio_write_32(0x508300d3a0,0x00000000);
	mmio_write_32(0x508300d3a4,0x00000000);
	mmio_write_32(0x508300d3a8,0x00000000);
	mmio_write_32(0x508300d3c0,0x00000001); //Active Memory Interleave Group Register
	mmio_write_32(0x508300d3c4,0x00000000); //Memory Interleave Function Select Register
	mmio_write_32(0x508300d400,0x81600000); //General Purpose Region Attribute Register
	mmio_write_32(0x508300d404,0x00080000); //General Purpose Region addr low 43:12
	mmio_write_32(0x508300d408,0x00000000); //General Purpose Region addr high 51:44
	mmio_write_32(0x508300dc00,0x00000000); //Credit
	mmio_write_32(0x508300dc04,0x00000000); //Credit
	mmio_write_32(0x508300dc08,0x00000000); //Credit
	mmio_write_32(0x508300dc0c,0x00000000); //Credit
	mmio_write_32(0x508300dc10,0x00000000); //Credit
	mmio_write_32(0x508300dc14,0x00000000); //Credit
	mmio_write_32(0x508300dc18,0x0000001f); //Credit
	mmio_write_32(0x508300dc1c,0x00000000); //Credit
	//dce7				base address:0x508300_e000
	mmio_write_32(0x508300e000,0x8000700e); //id
	mmio_write_32(0x508300e004,0x8000000e); //fab id
	mmio_write_32(0x508300e100,0x00000803);
	mmio_write_32(0x508300e104,0x00000001);
	mmio_write_32(0x508300e110,0x000003ff);
	mmio_write_32(0x508300e140,0x0000003f);
	mmio_write_32(0x508300e144,0x0000003f);
	mmio_write_32(0x508300e148,0x00000001);
	mmio_write_32(0x508300e190,0x00004000);
	mmio_write_32(0x508300e194,0x00004000);
	mmio_write_32(0x508300e200,0x001e0010); //QOS
	mmio_write_32(0x508300e3a0,0x00000000);
	mmio_write_32(0x508300e3a4,0x00000000);
	mmio_write_32(0x508300e3a8,0x00000000);
	mmio_write_32(0x508300e3c0,0x00000001); //Active Memory Interleave Group Register
	mmio_write_32(0x508300e3c4,0x00000000); //Memory Interleave Function Select Register
	mmio_write_32(0x508300e400,0x81600000); //General Purpose Region Attribute Register
	mmio_write_32(0x508300e404,0x00080000); //General Purpose Region addr low 43:12
	mmio_write_32(0x508300e408,0x00000000); //General Purpose Region addr high 51:44
	mmio_write_32(0x508300ec00,0x00000000); //Credit
	mmio_write_32(0x508300ec04,0x00000000); //Credit
	mmio_write_32(0x508300ec08,0x00000000); //Credit
	mmio_write_32(0x508300ec0c,0x00000000); //Credit
	mmio_write_32(0x508300ec10,0x00000000); //Credit
	mmio_write_32(0x508300ec14,0x00000000); //Credit
	mmio_write_32(0x508300ec18,0x00000000); //Credit
	mmio_write_32(0x508300ec1c,0x0000001f); //Credit
	//DMI_0_dmi			base address:0x508300_f000
	mmio_write_32(0x508300f000,0x8000000f); //id
	mmio_write_32(0x508300f004,0x8000000f); //fab id
	mmio_write_32(0x508300f100,0x00000033);
	mmio_write_32(0x508300f104,0x00000033);
	mmio_write_32(0x508300f108,0x00000001); //Uncorrectable Error Status Register
	mmio_write_32(0x508300f140,0x00000ff3);
	mmio_write_32(0x508300f144,0x00000001);
	mmio_write_32(0x508300f190,0x00004000);
	mmio_write_32(0x508300f194,0x00004000);
	mmio_write_32(0x508300f200,0x001e0010); //QOS
	mmio_write_32(0x508300f210,0x80000101); //QOS
	mmio_write_32(0x508300f800,0x10020000);
	mmio_write_32(0x508300fc00,0x40000000);
	mmio_write_32(0x508300fc08,0x40000000);
	mmio_write_32(0x508300fc10,0x40000000);
	mmio_write_32(0x508300fc18,0x40000000);
	//DMI_1_dmi			base address:0x508301_0000
	mmio_write_32(0x5083010000,0x80001010); //id
	mmio_write_32(0x5083010004,0x80000010); //fab id
	mmio_write_32(0x5083010100,0x00000033);
	mmio_write_32(0x5083010104,0x00000033);
	mmio_write_32(0x5083010108,0x00000001); //Uncorrectable Error Status Register
	mmio_write_32(0x5083010140,0x00000ff3);
	mmio_write_32(0x5083010144,0x00000001);
	mmio_write_32(0x5083010190,0x00004000);
	mmio_write_32(0x5083010194,0x00004000);
	mmio_write_32(0x5083010200,0x001e0010); //QOS
	mmio_write_32(0x5083010210,0x80000101); //QOS
	mmio_write_32(0x5083010800,0x10020000);
	mmio_write_32(0x5083010c00,0x40000000);
	mmio_write_32(0x5083010c08,0x40000000);
	mmio_write_32(0x5083010c10,0x40000000);
	mmio_write_32(0x5083010c18,0x40000000);
	//DMI_2_dmi			base address:0x508301_1000
	mmio_write_32(0x5083011000,0x80002011); //id
	mmio_write_32(0x5083011004,0x80000011); //fab id
	mmio_write_32(0x5083011100,0x00000033);
	mmio_write_32(0x5083011104,0x00000033);
	mmio_write_32(0x5083011108,0x00000001); //Uncorrectable Error Status Register
	mmio_write_32(0x5083011140,0x00000ff3);
	mmio_write_32(0x5083011144,0x00000001);
	mmio_write_32(0x5083011190,0x00004000);
	mmio_write_32(0x5083011194,0x00004000);
	mmio_write_32(0x5083011200,0x001e0010); //QOS
	mmio_write_32(0x5083011210,0x80000101); //QOS
	mmio_write_32(0x5083011800,0x10020000);
	mmio_write_32(0x5083011c00,0x40000000);
	mmio_write_32(0x5083011c08,0x40000000);
	mmio_write_32(0x5083011c10,0x40000000);
	mmio_write_32(0x5083011c18,0x40000000);
	//DMI_3_dmi			base address:0x508301_2000
	mmio_write_32(0x5083012000,0x80003012); //id
	mmio_write_32(0x5083012004,0x80000012); //fab id
	mmio_write_32(0x5083012100,0x00000033);
	mmio_write_32(0x5083012104,0x00000033);
	mmio_write_32(0x5083012108,0x00000001); //Uncorrectable Error Status Register
	mmio_write_32(0x5083012140,0x00000ff3);
	mmio_write_32(0x5083012144,0x00000001);
	mmio_write_32(0x5083012190,0x00004000);
	mmio_write_32(0x5083012194,0x00004000);
	mmio_write_32(0x5083012200,0x001e0010); //QOS
	mmio_write_32(0x5083012210,0x80000101); //QOS
	mmio_write_32(0x5083012800,0x10020000);
	mmio_write_32(0x5083012c00,0x40000000);
	mmio_write_32(0x5083012c08,0x40000000);
	mmio_write_32(0x5083012c10,0x40000000);
	mmio_write_32(0x5083012c18,0x40000000);
	//DMI_4_dmi			base address:0x508301_3000
	mmio_write_32(0x5083013000,0x80004013); //id
	mmio_write_32(0x5083013004,0x80000013); //fab id
	mmio_write_32(0x5083013100,0x00000033);
	mmio_write_32(0x5083013104,0x00000033);
	mmio_write_32(0x5083013108,0x00000001); //Uncorrectable Error Status Register
	mmio_write_32(0x5083013140,0x00000ff3);
	mmio_write_32(0x5083013144,0x00000001);
	mmio_write_32(0x5083013190,0x00004000);
	mmio_write_32(0x5083013194,0x00004000);
	mmio_write_32(0x5083013200,0x001e0010); //QOS
	mmio_write_32(0x5083013210,0x80000101); //QOS
	mmio_write_32(0x5083013800,0x10020000);
	mmio_write_32(0x5083013c00,0x40000000);
	mmio_write_32(0x5083013c08,0x40000000);
	mmio_write_32(0x5083013c10,0x40000000);
	mmio_write_32(0x5083013c18,0x40000000);
	//DMI_5_dmi			base address:0x508301_4000
	mmio_write_32(0x5083014000,0x80005014); //id
	mmio_write_32(0x5083014004,0x80000014); //fab id
	mmio_write_32(0x5083014100,0x00000033);
	mmio_write_32(0x5083014104,0x00000033);
	mmio_write_32(0x5083014108,0x00000001); //Uncorrectable Error Status Register
	mmio_write_32(0x5083014140,0x00000ff3);
	mmio_write_32(0x5083014144,0x00000001);
	mmio_write_32(0x5083014190,0x00004000);
	mmio_write_32(0x5083014194,0x00004000);
	mmio_write_32(0x5083014200,0x001e0010); //QOS
	mmio_write_32(0x5083014210,0x80000101); //QOS
	mmio_write_32(0x5083014800,0x10020000);
	mmio_write_32(0x5083014c00,0x40000000);
	mmio_write_32(0x5083014c08,0x40000000);
	mmio_write_32(0x5083014c10,0x40000000);
	mmio_write_32(0x5083014c18,0x40000000);
	//DMI_6_dmi			base address:0x508301_5000
	mmio_write_32(0x5083015000,0x80006015); //id
	mmio_write_32(0x5083015004,0x80000015); //fab id
	mmio_write_32(0x5083015100,0x00000033);
	mmio_write_32(0x5083015104,0x00000033);
	mmio_write_32(0x5083015108,0x00000001); //Uncorrectable Error Status Register
	mmio_write_32(0x5083015140,0x00000ff3);
	mmio_write_32(0x5083015144,0x00000001);
	mmio_write_32(0x5083015190,0x00004000);
	mmio_write_32(0x5083015194,0x00004000);
	mmio_write_32(0x5083015200,0x001e0010); //QOS
	mmio_write_32(0x5083015210,0x80000101); //QOS
	mmio_write_32(0x5083015800,0x10020000);
	mmio_write_32(0x5083015c00,0x40000000);
	mmio_write_32(0x5083015c08,0x40000000);
	mmio_write_32(0x5083015c10,0x40000000);
	mmio_write_32(0x5083015c18,0x40000000);
	//DMI_7_dmi			base address:0x508301_6000
	mmio_write_32(0x5083016000,0x80007016); //id
	mmio_write_32(0x5083016004,0x80000016); //fab id
	mmio_write_32(0x5083016100,0x00000033);
	mmio_write_32(0x5083016104,0x00000033);
	mmio_write_32(0x5083016108,0x00000001); //Uncorrectable Error Status Register
	mmio_write_32(0x5083016140,0x00000ff3);
	mmio_write_32(0x5083016144,0x00000001);
	mmio_write_32(0x5083016190,0x00004000);
	mmio_write_32(0x5083016194,0x00004000);
	mmio_write_32(0x5083016200,0x001e0010); //QOS
	mmio_write_32(0x5083016210,0x80000101); //QOS
	mmio_write_32(0x5083016800,0x10020000);
	mmio_write_32(0x5083016c00,0x40000000);
	mmio_write_32(0x5083016c08,0x40000000);
	mmio_write_32(0x5083016c10,0x40000000);
	mmio_write_32(0x5083016c18,0x40000000);
	//sys_dii			base address:0x508301_7000
	mmio_write_32(0x5083017000,0x80000017); //id
	mmio_write_32(0x5083017004,0x80000017); //fab id
	mmio_write_32(0x5083017118,0x00000013);
	mmio_write_32(0x508301711c,0x00000003);
	mmio_write_32(0x5083017120,0x00000001);
	mmio_write_32(0x5083017190,0x00004000);
	mmio_write_32(0x5083017194,0x00004000);
	mmio_write_32(0x5083017900,0x10020000);
	mmio_write_32(0x5083017c00,0x40000000);
	mmio_write_32(0x5083017c08,0x40000000);
	mmio_write_32(0x5083017c10,0x40000000);
	mmio_write_32(0x5083017c18,0x40000000);
	//dve0				base address:0x508301_8000
	mmio_write_32(0x5083018000,0x80000018); //id
	mmio_write_32(0x5083018004,0x80000018); //fab id
	mmio_write_32(0x5083018100,0x00000ff3);
	mmio_write_32(0x5083018108,0x00000001);
	mmio_write_32(0x5083018140,0x0000001f);
	mmio_write_32(0x5083018144,0x0000001f);
	mmio_write_32(0x5083018148,0x00000001);
	mmio_write_32(0x5083018180,0x00000001);
	mmio_write_32(0x5083018190,0x00004000);
}

