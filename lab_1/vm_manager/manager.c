#include "manager.h"

int kvmfd ; // kvm subsys file desc
int vmfd ; //  on virtual machine file desc
int vcpufd ; // vCPU file desc

struct kvm_run *run;
uint8_t *memory;
int slot_id = 0;

/***
 * TODO
 * This Function Creates Virtual Machine File Descriptor.
 */
int create_vm( void ) {
    int ret = 0;

    kvmfd = open( "/dev/kvm" , O_RDWR ) ; // obtains a handle to the kvm subsystem
    if ( kvmfd < 0 ) { // Error in the open function
        printf("Error in initial file open.\n") ;
        return -1 ;
    }
    vmfd = ioctl( kvmfd , KVM_CREATE_VM , 0 ) ; // Create the VM
    if ( vmfd < 0 ){
        ret = -1 ;
    }
    return ret;
}

/***
 * TODO
 * This Function Creates Guest Physical Memory Based On Host Virtual Memory
 * And Submits The Memory Area To KVM.
 */
int create_guest_physical_memory( size_t size ) {
    int ret = 0 ;

    // First we map the virtual HVA to HPA
    // void *mmap( void addr[ .length ] , size_t length , int prot , int flags , int fd , off_t offset ) :
    //      creates a new mapping in the virtual address space of the calling process.  The starting address for the new mapping is specified in addr.  
    //      The length argument specifies the length of the mapping (which must be greater than 0).
    
    // NULL : The kernel chooses the (page-aligned) address at which to create the mapping
    void* hva = mmap( NULL , size , PROT_READ | PROT_WRITE , MAP_ANONYMOUS | MAP_PRIVATE , -1 , 0 ) ;
    if ( hva == MAP_FAILED ){ // Error check
        ret = -1 ;
    }
    memory = hva ; 

    // Fill in structure's fields
    struct kvm_userspace_memory_region gpa ;
    gpa.memory_size = size ;
    gpa.slot = slot_id ;
    gpa.userspace_addr = (uint64_t)hva ;
    gpa.guest_phys_addr = 0 ;
    gpa.flags = 0 ;
    ret = ioctl ( vmfd , KVM_SET_USER_MEMORY_REGION , &gpa ) ;

    return ret ;
}

int create_bootstrap() {
    struct kvm_sregs sregs;
    size_t mmap_size;
    int ret = -1;
    int nent = 128;
    struct kvm_cpuid2 *cpuid2 = (struct kvm_cpuid2 *)malloc(sizeof(struct kvm_cpuid2) + nent * sizeof(struct kvm_cpuid_entry2));
    cpuid2->nent = nent;
    if (ioctl(kvmfd, KVM_GET_SUPPORTED_CPUID, cpuid2) < 0)
        err(1, "cant get cpuid");

    vcpufd = ioctl(vmfd, KVM_CREATE_VCPU, (unsigned long)0);

    if (vcpufd == -1)
        err(1, "Cannot create vcpu\n");

    if (ioctl(vcpufd, KVM_SET_CPUID2, cpuid2) < 0)
        err(1, "cannot set cpuid things\n");

    ret = ioctl(kvmfd, KVM_GET_VCPU_MMAP_SIZE, NULL);
    if (ret == -1)
        err(1, "KVM_GET_VCPU_MMAP_SIZE");
    mmap_size = ret;
    if (mmap_size < sizeof(*run))
        errx(1, "KVM_GET_VCPU_MMAP_SIZE unexpectedly small");
    run = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, vcpufd, 0);
    if (!run)
        err(1, "mmap vcpu");

    ret = ioctl(vcpufd, KVM_GET_SREGS, &sregs);
    if (ret == -1)
        err(1, "KVM_GET_SREGS");
    sregs.cs.base = 0;
    sregs.cs.selector = 0;
    ret = ioctl(vcpufd, KVM_SET_SREGS, &sregs);
    if (ret == -1)
        err(1, "KVM_SET_SREGS");
    return ret;
}

/***
 * TODO
 * This Function Updates vCPU Registers And Runs It.
 */
int launch_vm(){
    int ret;
    struct kvm_regs regs;
    ioctl( vcpufd , KVM_GET_REGS , &regs ) ;

    /* Update Of The vCPU Registers - RAX, RBX and RIP */
    regs.rflags = 2;
    regs.rax = 4 ; // Accumulator register (AX).
    regs.rbx = 2 ; // Base register (BX) : Used as a pointer to data (located in segment register DS, when in segmented mode).

    /*  We can use 0x1000 because the first page sometimes is reserved / used for other stuff (like vectors, structures, etc.), 
        but not always. We should then change the for loop in the code that loads the VM.
    */
    regs.rip = 0 ; // 0x1000 ; // Index Pointer : Holds the offset of the next instruction (code is loaded at 0x0000)

    ret = ioctl(vcpufd, KVM_SET_REGS, &regs);
    if (ret == -1)
        err(1, "KVM_SET_REGS");

    while (1)
    {

        if (ioctl(vcpufd, KVM_RUN, NULL) == -1)
            err(1, "KVM_RUN");
        else
        {
            if (vmexit_handler(run->exit_reason) == 0)
                break;
        }
    }
    return 0;
}

int vmexit_handler(int exit_reason)
{
    struct kvm_regs regs;
    struct kvm_sregs sregs;
    struct kvm_fpu fpu;
    ioctl(vcpufd, KVM_GET_FPU, &fpu);
    ioctl(vcpufd, KVM_GET_REGS, &regs);
    ioctl(vcpufd, KVM_GET_SREGS, &sregs);
    switch (exit_reason)
    {
    case KVM_EXIT_HLT:
        printf("KVM_EXIT_HLT\n");
        return 0;
    case KVM_EXIT_IO:
        if (run->io.direction == KVM_EXIT_IO_OUT && run->io.size == 1 && run->io.port == 0x3f8 && run->io.count == 1)
            putchar(*(((char *)run) + run->io.data_offset));
        else
            errx(1, "unhandled KVM_EXIT_IO");
        break;
    case KVM_EXIT_FAIL_ENTRY:
        errx(1, "KVM_EXIT_FAIL_ENTRY: hardware_entry_failure_reason = 0x%llx",
             (unsigned long long)run->fail_entry.hardware_entry_failure_reason);
    case KVM_EXIT_INTERNAL_ERROR:
        errx(1, "KVM_EXIT_INTERNAL_ERROR: suberror = 0x%x RIP %llx", run->internal.suberror, regs.rip);
    case KVM_EXIT_SHUTDOWN:
        errx(1, "SHUTDOWN RIP %llx - RSP %llx - RAX %llx\n", regs.rip, regs.rsp, regs.rax);
        break;
    default:
        errx(1, "exit_reason = 0x%d rip %llx, rax %llx", run->exit_reason, regs.rip, regs.rax);
    }
    return 1;
}

uint8_t *get_memory()
{
    return memory;
}