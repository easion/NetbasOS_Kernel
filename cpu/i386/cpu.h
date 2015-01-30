
#ifndef     PROCESSOR_H
#define    PROCESSOR_H

#define MAX_INTEL_MODEL 5

typedef struct 
{
  int cpuid_levels;  /* Maximum supported CPUID level, -1=no CPUID */

  int fpu;
  int cpuid;

  unsigned long eax_2[4];
  unsigned long serial[3];
  unsigned long brand_id;
  unsigned long family_ex:12;

  unsigned char type;
  unsigned char family;
  unsigned char model;

  unsigned char model_ex;
  unsigned char setpping;

 // unsigned long model_ex:4;
//  unsigned long setpping:4;

  unsigned long speed;
  long flags;
  long flags_ex;

	char	x86_vendor_id[16];
    char vendor_family[32];
	char	x86_model_id[64];
	int 	x86_cache_size;  /* in KB - valid for CPUS which support this call */

}cpuinfo_t;


typedef struct {
	int m_family;
	char *m_name[16];
}cpu_model_info_t;

   cpu_model_info_t cpu_models[ MAX_INTEL_MODEL ] = {
	{ 	3,
	  { "386", "386", "UNKNOW INTEL TYPE", "386",
	    "UNKNOW INTEL TYPE", "386", "386",
	    "386", "386", "UNKNOW INTEL TYPE",
	    "386", "UNKNOW INTEL TYPE", "UNKNOW INTEL TYPE", "UNKNOW INTEL TYPE",
		"UNKNOW INTEL TYPE" }},
	{ 	4,
	  { "486 DX-25/33", "486 DX-50", "486 SX", "486 DX/2", "486 SL",
	    "486 SX/2", "UNKNOW INTEL TYPE", "486 DX/2-WB", "486 DX/4", "486 DX/4-WB", 
		"UNKNOW INTEL TYPE", "UNKNOW INTEL TYPE", "UNKNOW INTEL TYPE", 
		"UNKNOW INTEL TYPE", "UNKNOW INTEL TYPE", "UNKNOW INTEL TYPE" }},
	{ 	5,
	  { "Pentium 60/66 A-step", "Pentium 60/66", "Pentium 75 - 200",
	    "OverDrive PODP5V83", "Pentium MMX", "UNKNOW INTEL TYPE", "UNKNOW INTEL TYPE",
	    "Mobile Pentium 75 - 200", "Mobile Pentium MMX", "UNKNOW INTEL TYPE",
		"UNKNOW INTEL TYPE", "UNKNOW INTEL TYPE",
	    "UNKNOW INTEL TYPE", "UNKNOW INTEL TYPE", "UNKNOW INTEL TYPE", 
		"UNKNOW INTEL TYPE" }},
	{ 	6,
	  { "Pentium Pro A-step", "Pentium Pro", "UNKNOW INTEL TYPE", "Pentium II (Klamath)",
	    "UNKNOW INTEL TYPE", "Pentium II (Deschutes)", "Mobile Pentium II",
	    "Pentium III (Katmai)", "Pentium III (Coppermine)", "UNKNOW INTEL TYPE",
	    "Pentium III (Cascades)", "UNKNOW INTEL TYPE", "UNKNOW INTEL TYPE", 
		"UNKNOW INTEL TYPE", "UNKNOW INTEL TYPE" }},
	{ 	7,
	  { "PenTIUM", "Pentium ", "UNKNOW INTEL TYPE", "Pentium",
	    "UNKNOW INTEL TYPE", "Pentium ", "Pentium",
	    "Pentium ", "Pentium ", "UNKNOW INTEL TYPE",
	    "Pentium ", "UNKNOW INTEL TYPE", "UNKNOW INTEL TYPE", "UNKNOW INTEL TYPE", 
		"UNKNOW INTEL TYPE" }},

};

#endif

