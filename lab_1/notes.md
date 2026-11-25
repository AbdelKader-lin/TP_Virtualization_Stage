# ScratchVM: Lab 1
# EL HOUSSAMI Abdel Kader - MOSIG 1


## Step 0: Taking control of KVM 

Notes : ioctl stands for "input/output control" - it's a Linux system call used to 
communicate with device drivers and special files.

* Needed ioctls for:
    * Creating a virtual machine and virtual CPU :

        -> To create a virtual machine : KVM_CREATE_VM
        . Parameters: machine type identifier (KVM_VM_*)
        . Returns: a VM fd that can be used to control the new virtual machine.

            The new VM has no virtual cpus and no memory.

        -> To create a vCPU : KVM_CREATE_VCPU
        . Parameters: vcpu id (apic id on x86)
        . Returns: vcpu fd on success, -1 on error

            This API adds a vcpu to a virtual machine. No more than max_vcpus may be added.
            The vcpu id is an integer in the range [ 0 , max_vcpu_id ].


    * Accessing the virtual CPU registers :
        1- KVM_GET_REGS :
            Reads the general purpose registers from the vcpu.
        
        2- KVM_SET_REGS :
            Writes the general purpose registers into the vcpu.
        
        3- KVM_GET_SREGS :
            Reads special registers from the vcpu.

        4- KVM_SET_SREGS :
            Writes special registers into the vcpu.

        5- KVM_TRANSLATE :
            Translates a virtual address according to the vcpu's 
            current address translation mode.

        6- KVM_GET_MSRS :
            When used as a vcpu ioctl:
                Reads model-specific registers from the vcpu.  Supported msr indices 
                can be obtained using KVM_GET_MSR_INDEX_LIST in a system ioctl.
        
        7- KVM_SET_MSRS :
            Writes model-specific registers to the vcpu.


    * Allocating guest physical memory :

        1- KVM_GET_VCPU_MMAP_SIZE :
            Returns: size of vcpu mmap area, in bytes

        2- KVM_SET_USER_MEMORY_REGION :
            This ioctl allows the user to create, modify or delete a guest physical memory slot. Bits 0-15 of "slot" specify the slot id and this value should be less than the maximum number of user memory slots supported per VM. The maximum allowed slots can be queried using KVM_CAP_NR_MEMSLOTS. Slots may not overlap in guest physical address space.

            If KVM_CAP_MULTI_ADDRESS_SPACE is available, bits 16-31 of "slot"
            specifies the address space which is being modified

        3- KVM_SET_TSS_ADDR :
            This ioctl defines the physical address of a three-page region in the guest
            physical address space.  The region must be within the first 4GB of the
            guest physical address space and must not conflict with any memory slot
            or any mmio address.  The guest may malfunction if it accesses this memory
            region.


    * Running the VM :
        1- KVM_RUN :
            This ioctl is used to run a guest virtual cpu.

