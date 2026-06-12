#include <stdio.h>

#define MAX_PRINTED_ERRORS 10

#define STRELA_CTRL_BIT_START_EXEC    	    0x1
#define STRELA_CTRL_BIT_CLEAR_STATE   	    0x2

#define STRELA_CTRL_BIT_LOAD_CONFIG   	    0x4
#define STRELA_CTRL_BIT_CLEAR_CONFIG	    0x8

#define STRELA_CTRL_BIT_CLEAR_INT_CONFIG    0x10
#define STRELA_CTRL_BIT_CLEAR_INT_EXEC	    0x20

#define STRELA_CTRL_BIT_DONE_CONFIG	        0x2
#define STRELA_CTRL_BIT_DONE_EXEC	        0x1
#define STRELA_CTRL_BIT_PENDING_INT_CONFIG	0x4
#define STRELA_CTRL_BIT_PENDING_INT_EXEC	0x8

// register offsets
#define STRELA_CTRL_A 		    0x00

#define STRELA_CONF_ADDR_A	    0x04
#define STRELA_CONF_SIZE_A	    0x08

#define STRELA_IN0_ADDR_A	    0x10
#define STRELA_IN0_SIZE_A	    0x14
#define STRELA_IN1_ADDR_A	    0x18
#define STRELA_IN1_SIZE_A	    0x1C
#define STRELA_IN2_ADDR_A	    0x20
#define STRELA_IN2_SIZE_A	    0x24
#define STRELA_IN3_ADDR_A	    0x28
#define STRELA_IN3_SIZE_A	    0x2C

#define STRELA_OUT0_ADDR_A	    0x50
#define STRELA_OUT0_SIZE_A	    0x54
#define STRELA_OUT1_ADDR_A	    0x58
#define STRELA_OUT1_SIZE_A	    0x5C
#define STRELA_OUT2_ADDR_A	    0x60
#define STRELA_OUT2_SIZE_A	    0x64
#define STRELA_OUT3_ADDR_A	    0x68
#define STRELA_OUT3_SIZE_A	    0x6C

#define STRELA_CNTR_CONF_A    	0x90
#define STRELA_CNTR_EXEC_A    	0x94
#define STRELA_CNTR_STALL_A   	0x98

#define STRELA_OUT_ARB_HOLD_A 	0xA0

#define STRELA_IN0_STRIDE_A     0xA4
#define STRELA_IN1_STRIDE_A     0xA8
#define STRELA_IN2_STRIDE_A     0xAC
#define STRELA_IN3_STRIDE_A     0xB0

#define STRELA_RESET_DMA_A      0xF8

#define STRELA_AM_OPA 		    0xF0
#define STRELA_AM_OPB 		    0xF4
#define STRELA_AM_OPR 		    0xF8

#define BYPASS_NPE      (16)
#define BYPASS_SIZE     (BYPASS_NPE * 6)
#define BYPASS_BYTES    (BYPASS_SIZE * sizeof(unsigned))

#define BUFF_SIZE (100)

#define TILE_NUM (2) // same as in config

#define CSR_TILE_SIZE    0x200
#define CSR_BASE_ADDR    0x60090000
#define COH_REG_INDEX    99
#define TP_RST_REG_INDEX 100

typedef long token_t;
typedef long native_t;

enum accelerator_coherence {
    ACC_COH_NONE = 0,
    ACC_COH_LLC,
    ACC_COH_RECALL,
    ACC_COH_FULL,
    ACC_COH_AUTO
};

struct esp_device {
    //unsigned vendor;
    //unsigned id;
    //unsigned number;
    //unsigned irq;
    long long unsigned addr;
    //unsigned compat;
    //char name[DEVNAME_MAX_LEN];
};

unsigned bypass_kernel[BYPASS_SIZE] = {
    0x00000000, 0x00000021, 0x00000000, 0x00000000, 0x00000000, 0x00000000, // 12
    0x00000000, 0x00000021, 0x00000000, 0x00000000, 0x00000000, 0x00000000, // 8
    0x00000000, 0x00000021, 0x00000000, 0x00000000, 0x00000000, 0x00000000, // 4
    0x00000000, 0x00000021, 0x00000000, 0x00000000, 0x00000000, 0x00000000, // 0

    0x00000000, 0x00000021, 0x00000000, 0x00000000, 0x00000000, 0x00000000, // 13
    0x00000000, 0x00000021, 0x00000000, 0x00000000, 0x00000000, 0x00000000, // 9
    0x00000000, 0x00000021, 0x00000000, 0x00000000, 0x00000000, 0x00000000, // 5
    0x00000000, 0x00000021, 0x00000000, 0x00000000, 0x00000000, 0x00000000, // 1

    0x00000000, 0x00000021, 0x00000000, 0x00000000, 0x00000000, 0x00000000, // 14
    0x00000000, 0x00000021, 0x00000000, 0x00000000, 0x00000000, 0x00000000, // 10
    0x00000000, 0x00000021, 0x00000000, 0x00000000, 0x00000000, 0x00000000, // 6
    0x00000000, 0x00000021, 0x00000000, 0x00000012, 0x00000000, 0x00000000, // 2

    0x00000000, 0x00000021, 0x00000000, 0x00000000, 0x00000000, 0x00000000, // 15
    0x00000000, 0x00000021, 0x00000000, 0x00000000, 0x00000000, 0x00000000, // 11
    0x00000000, 0x00000021, 0x00000000, 0x00000000, 0x00000000, 0x00000000, // 7
    0x00000000, 0x00000021, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // 3
};

static token_t mem[BUFF_SIZE];
static token_t mem_out[BUFF_SIZE];
static native_t gold[BUFF_SIZE];

static int validate_buf(token_t *out, native_t *gold)
{
    int j;
    native_t val;
    unsigned errors = 0;

    for (j = 0; j < BUFF_SIZE; j++) {
        val = out[j];

        if (gold[j] != val) {
            errors++;
            if (errors <= MAX_PRINTED_ERRORS) { printf("%d : %d : %d\n", j, val, gold[j]); }
        }
    }

    return errors;
}

static void init_buf(token_t *in, native_t *gold)
{
    // initialize input data
    for(int i = 0; i < BUFF_SIZE; i++)
    {
        in[i] = i;
    }

    for(int i = 0; i < BUFF_SIZE; i++)
    {
        gold[i] = in[i];
    }
}

unsigned ioread32(struct esp_device *dev, unsigned offset)
{
    const long unsigned addr = dev->addr + offset;
    volatile unsigned *reg   = (unsigned *)addr;
    return *reg;
}

void iowrite32(struct esp_device *dev, unsigned offset, unsigned payload)
{
    const long unsigned addr = dev->addr + offset;
    volatile unsigned *reg   = (unsigned *)addr;
    *reg                     = payload;
}

int main(int argc, char *argv[])
{
    struct esp_device dev_esp;
    dev_esp.addr = 0x60400000; // based on the device tree when generating soc for a specific fpga board
    struct esp_device *dev = &dev_esp;
    unsigned done;
    unsigned errors = 0;

    printf("  Generate input...\n");

    init_buf(mem, gold);

    // test write/read to a scratch register first
    iowrite32(dev, STRELA_AM_OPA, 0xcafe);

    int read_val = ioread32(dev, STRELA_AM_OPA);
    if (read_val != 0xcafe)
    {
        printf("error when writting and reading a value to/from a scratch register\n");
        return 0;
    }


    unsigned int coh;
    unsigned int tile_offset;
    unsigned int *coh_reg_addr;
    unsigned int *rst_reg_addr;

    tile_offset   = (CSR_TILE_SIZE / sizeof(unsigned int)) * TILE_NUM;
    coh_reg_addr  = ((unsigned int *)CSR_BASE_ADDR) + tile_offset + COH_REG_INDEX;
    rst_reg_addr  = ((unsigned int *)CSR_BASE_ADDR) + tile_offset + TP_RST_REG_INDEX;
    coh           = ACC_COH_RECALL;
    *coh_reg_addr = coh;

    for(int i = 0; i < BUFF_SIZE; i++)
    {
        mem_out[i] = -1;
    }

    unsigned *cgra_kernel = bypass_kernel;

    iowrite32(dev, STRELA_CONF_ADDR_A, (unsigned)cgra_kernel);
    iowrite32(dev, STRELA_CONF_SIZE_A, BYPASS_BYTES);

    iowrite32(dev, STRELA_IN0_ADDR_A, (unsigned)mem);
    iowrite32(dev, STRELA_IN0_SIZE_A, BUFF_SIZE * sizeof(token_t));

    iowrite32(dev, STRELA_IN1_ADDR_A, 0);
    iowrite32(dev, STRELA_IN1_SIZE_A, 0);

    iowrite32(dev, STRELA_IN2_ADDR_A, 0);
    iowrite32(dev, STRELA_IN2_SIZE_A, 0);

    iowrite32(dev, STRELA_IN3_ADDR_A, 0);
    iowrite32(dev, STRELA_IN3_SIZE_A, 0);

    iowrite32(dev, STRELA_OUT0_ADDR_A, (unsigned)mem_out);
    iowrite32(dev, STRELA_OUT0_SIZE_A, BUFF_SIZE * sizeof(token_t));

    iowrite32(dev, STRELA_OUT1_ADDR_A, 0);
    iowrite32(dev, STRELA_OUT1_SIZE_A, 0);

    iowrite32(dev, STRELA_OUT2_ADDR_A, 0);
    iowrite32(dev, STRELA_OUT2_SIZE_A, 0);

    iowrite32(dev, STRELA_OUT3_ADDR_A, 0);
    iowrite32(dev, STRELA_OUT3_SIZE_A, 0);

    iowrite32(dev, STRELA_IN0_STRIDE_A, sizeof(token_t));
    iowrite32(dev, STRELA_IN1_STRIDE_A, 0);
    iowrite32(dev, STRELA_IN2_STRIDE_A, 0);
    iowrite32(dev, STRELA_IN3_STRIDE_A, 0);

    iowrite32(dev, STRELA_CTRL_A, STRELA_CTRL_BIT_CLEAR_CONFIG);
    iowrite32(dev, STRELA_RESET_DMA_A, 1);

    iowrite32(dev, STRELA_CTRL_A, STRELA_CTRL_BIT_LOAD_CONFIG);

    // wait for config done
    while(!(ioread32(dev, STRELA_CTRL_A) & STRELA_CTRL_BIT_DONE_CONFIG)){};

    iowrite32(dev, STRELA_CTRL_A, STRELA_CTRL_BIT_CLEAR_STATE);

    printf("  Done config\n");

    iowrite32(dev, STRELA_CTRL_A, STRELA_CTRL_BIT_START_EXEC);

    // wait for exec done
    while(!(ioread32(dev, STRELA_CTRL_A) & STRELA_CTRL_BIT_DONE_EXEC)){};

    printf("  Done exec\n");

    printf("  Validating results\n");

    for(int i = 0; i < BUFF_SIZE; i++)
    {
        printf("%d\n", mem_out[i]);
    }

    errors += validate_buf(mem_out, gold);

    if (errors) printf("  ... FAIL\n");
    else
        printf("  ... PASS\n");

   return 0;
}
