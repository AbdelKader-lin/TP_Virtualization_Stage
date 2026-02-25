# ScratchVM - Lab 3
## VM Migration (Cold Migration)

---

### 📋 Lab Overview

**Objective:** Implement a simpler version of cold migration for a micro virtual machine

**VM Configuration:**
- 1 vCPU
- Low amount of physical memory
- No devices (disk, network, peripherals)

**Migration Type:** Cold Migration (not live migration)

---

## Background: VM Migration

### 🔹 What is VM Migration?

**Definition:**
VM migration is a mechanism performed by the hypervisor that consists of moving a virtual machine from a physical machine to another.

**Use Cases:**
1. **Maintenance:** Migrate VMs before shutting down a server for maintenance
2. **Performance:** Gather VMs that communicate with each other on the same physical machine to reduce latencies
3. **Power Saving & Consolidation:** Maximize efficiency by gathering VMs on fewer machines and turning off empty ones

---

### 🔹 Types of Migration

#### Cold Migration
- **Stop** the target VM
- **Move** it to the target physical machine
- **Resume** its execution
- Applications are held during migration (higher latency)

#### Live Migration
- VM **still running** on source during migration
- Shorter latency than cold migration
- **Not covered in this lab**

---

### 🔹 Cold Migration Operations

#### SAVE Operation
1. Pause all VM **vCPUs**
2. Dump the **guest physical memory**
3. Dump **vCPU registers** (privileged and unprivileged)
4. Dump **devices** (file descriptors)
5. Dump **pending IO requests**
6. **Send** (network) the dumped information to the VMM on target physical machine

#### RESTORE Operation
1. Create a **blank VM**
2. **Update** the blank VM state with the saved image file
3. Run the vCPUs

---

## Step 1: VM State and Image File

### 🔹 Task 1.1: List VM Components

**Question:** List all the VM components (vCPU registers, physical memory, device state, IO pending requests, ...) that can be part of the deployed VM state.

**Answer:**

---

### 🔹 Task 1.2: Describe VM Image Format

**Question:** Describe your VM image format.

**Answer:**

**VM Image Structure:**
```
+---------------------------+
| vCPU General Registers    |  (struct kvm_regs)
|   - rax, rbx, rcx, rdx    |
|   - rsi, rdi, rsp, rbp    |
|   - r8-r15, rip, rflags   |
+---------------------------+
| vCPU Special Registers    |  (struct kvm_sregs)
|   - CS, DS, SS, ES, etc.  |
|   - CR0, CR3, CR4         |
|   - EFER, GDT, IDT        |
+---------------------------+
| Memory Size               |  (size_t)
+---------------------------+
| Guest Physical Memory     |  (raw bytes)
|   - All memory content    |
+---------------------------+
```

---

### 🔹 Task 1.3: Implement dump() Functions

**Question:** Implement a dump() function for each component. For vCPU registers, check boot.asm to identify all different register types used.

**Answer:**

**Registers Used in boot.asm:**
- **General Purpose:** eax, ecx, edx, esp, ebp, r11, r15, r14
- **Control Registers:** cr0, cr3, cr4
- **Segment Registers:** cs, ds, es, ss, fs, gs
- **Model-Specific Registers (MSR):** EFER (0xC0000080), LSTAR (0xC0000082), STAR (0xC0000081)
- **Special:** rflags, rip, gdtr

**Implementation Notes:**


---

**---------------------------------------------------------------------**


## Step 2: SAVE Operation

### 🔹 Task 2.1: Implement SAVE() Function

**Question:** Implement the SAVE() function that triggers a VM exit to stop the vCPU.

**Reference:** [app.c:21](../lab_3/vm_src/src/app.c)

**Answer:**

**Implementation Strategy:**
- Use `vmcall` instruction to trigger a hypercall (VM exit)
- This will cause a `KVM_EXIT_HYPERCALL` exit reason

**Code:**
```c
void SAVE()
{
    // TODO: Trigger VM exit using vmcall
    __asm__ volatile("vmcall");
}
```

---

### 🔹 Task 2.2: Catch and Handle SAVE VM Exit

**Question:** Catch and handle this VM exit by performing the VM state dump.

**Answer:**

**VM Exit Handler Modification:**
- Add case for `KVM_EXIT_HYPERCALL` in `vmexit_handler()`
- Call `save_vm_state()` function
- Dump all VM components to disk

**Implementation Steps:**


---

### 🔹 Task 2.3: Save VM State and Exit VMM

**Question:** Save the VM state dump on the disk and exit the VMM.

**Expected Output:**
```bash
$ cd vm_src/
$ make save
./bin/save
OPEN ./ay_caramba 32834 511 - return 8
WRITE 8 a07e 16 - return 16
SAVE OPERATION
$ 
```

**Answer:**

**Implementation Details:**


---

**---------------------------------------------------------------------**


## Step 3: RESTORE Operation

### 🔹 Task 3.1: Create Blank VM

**Question:** Create a main file (restore.c) that creates a blank VM.

**Answer:**

**Steps:**
1. Create VM structure (`create_vm()`)
2. Create vCPU (`create_bootstrap()`)
3. Allocate memory (`add_memory()`)
4. Create stack (`create_stack()`)

---

### 🔹 Task 3.2: Read VM Image File

**Question:** Read the content of the VM image file and update the VM components.

**Answer:**

**Implementation:**


---

### 🔹 Task 3.3: Launch Restored VM

**Question:** Launch the VM (run the vCPU) from the restored state.

**Expected Output:**
```bash 
$ cd vm_src/
$ make restore
./bin/restore
RESTORE OPERATION
WRITE 8 a073 12 - return 12
CLOSE 8 - return 0
EXIT 0
$ 
```

**Answer:**

**Final Steps:**


---

**---------------------------------------------------------------------**


## Bonus: Realistic Migration (Client-Server)

### 🔹 Bonus Task: Implement Network Migration

**Question:** Implement migration between VMM_client and VMM_server using network communication.

**Expected Output:**

**VMM Client Terminal:**
```bash
$ make save
VM_APP - OPEN ./ay_caramba 32834 511 - return 8
VM_APP - WRITE 8 a07e 16 - return 16
VM_APP - SAVE OPERATION
VM_APP - VM STATE SENT
VMM EXITING
```

**VMM Server Terminal:**
```bash
$ make restore
VM_APP - RESTORE OPERATION
VM_APP - WRITE 8 a073 12 - return 12
VM_APP - CLOSE 8 - return 0
VM_APP - EXIT 0
```

**File Content Verification:**
```bash
$ cd vm_src
$ cat ./ay_caramba.txt
Hello World !!!
Bye Bye !!!
```

**Answer:**

**Implementation Strategy:**


---

**---------------------------------------------------------------------**


## 📝 Technical Notes

### KVM APIs Used

**For SAVE:**
- `KVM_GET_REGS` - Get general purpose registers
- `KVM_GET_SREGS` - Get special registers (segment, control registers)
- `KVM_EXIT_HYPERCALL` - VM exit reason for vmcall instruction

**For RESTORE:**
- `KVM_SET_REGS` - Set general purpose registers
- `KVM_SET_SREGS` - Set special registers
- `KVM_RUN` - Resume VM execution

---

### Key Concepts

**vmcall Instruction:**
- Hypercall instruction that triggers a VM exit
- Causes transition from guest to host (VMM)
- Exit reason: `KVM_EXIT_HYPERCALL`

**VM Image File:**
- Contains complete VM state snapshot
- Enables VM to resume from exact same point
- Binary format for efficiency

**Identity Page Table:**
- Guest virtual address (GVA) = Guest physical address (GPA)
- Simplifies address translation
- GPA → Host virtual address (HVA) = memory_base + GPA

---

### Important File References

**Source Files:**
- [app.c](vm_src/src/app.c) - VM application with SAVE call
- [boot.asm](vm_src/src/boot.asm) - Bootstrap code showing register usage
- [save.c](vm_src/src/save.c) - Main file for SAVE operation
- [restore.c](vm_src/src/restore.c) - Main file for RESTORE operation

**Manager Files:**
- [manager.c](vm_manager/manager.c) - VM management and exit handling
- [manager.h](vm_manager/manager.h) - VM structures and function declarations
- [syscall_handler.c](syscall_manager/syscall_handler.c) - System call handling

---

### 📚 Additional Resources

- [KVM API Documentation](https://www.kernel.org/doc/Documentation/virtual/kvm/api.txt)
- [Intel x86_64 Manual - VMCALL Instruction](https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-instruction-set-reference-manual-325383.html)
- [Linux System Call Table](https://blog.rchapman.org/posts/Linux_System_Call_Table_for_x86_64/)

---

### 🐛 Debugging Tips

1. **Check VM exit reasons:** Print exit_reason to understand why VM exited
2. **Verify register values:** Print registers before/after save/restore
3. **Memory dumps:** Compare memory content before save and after restore
4. **File permissions:** Ensure VM image file has proper read/write permissions
5. **Binary mode:** Always open VM image file in binary mode (wb/rb)

---

### 📊 Testing Checklist

- [ ] SAVE operation triggers VM exit correctly
- [ ] VM state is saved to disk (vm_image.bin created)
- [ ] VM image file contains all necessary components
- [ ] RESTORE operation reads VM image successfully
- [ ] Restored VM resumes from correct point
- [ ] Output file (ay_caramba.txt) contains expected content
- [ ] VMM exits cleanly after SAVE
- [ ] No memory leaks or segmentation faults

---
