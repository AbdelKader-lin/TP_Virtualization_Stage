# KVM Virtualization Labs

A set of low-level virtualization labs built around Linux KVM. The exercises progressively explore how a virtual machine monitor creates and runs a small virtual machine, handles VM exits and preserves VM state for migration.

## Overview

The repository contains three labs:

### Lab 1: Creating a virtual machine

Introduction to KVM and the basic components required to create and execute a small virtual machine.

### Lab 2: VM execution and exits

Runs a C application inside the virtual machine and explores the interaction between guest execution and the virtual machine monitor, including VM exits.

### Lab 3: Cold VM migration

Implements a simplified cold migration mechanism. The VM is stopped, its state is saved to an image, and a new VM instance restores that state before execution resumes.

The migration work involves preserving guest memory and vCPU state and rebuilding the VM from the saved state. The lab focuses on the SAVE and RESTORE operations used by virtual machine monitors.

## Repository structure

```text
lab_1/   VM creation and basic execution
lab_2/   Guest application and VM exit handling
lab_3/   VM state save/restore and cold migration
```

Each lab contains its own `INSTRUCTIONS.md`. The first labs also contain answer files documenting the exercises.

## Main concepts

The project covers Linux KVM, virtual CPUs, guest physical memory, VM exits, guest/host interaction, virtual machine monitors, VM state serialization and cold migration.

## Context

These labs were used to study virtualization from the hypervisor side rather than through high-level VM management tools. The objective was to understand what happens underneath a virtual machine, from creating the vCPU and guest memory to saving and restoring execution state.