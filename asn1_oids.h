/*----------------------------------------------------------------------------------------*\

	! BALANCE SOFTWARE CONFIDENTIAL !
	
	Copyright (c) 2005 Balance Software Corporation
	All Rights Reserved.
	
	NOTICE:
	
		All information contained herein is, and remains the property of,
		Balance Software Corporation and its suppliers, if any.  The
		intellectual and technical concepts contained herein are proprietary
		to Balance Software Corporation and its suppliers and may be covered
		by U.S. and foreign patents or patents in process, and are protected
		by trade secret and copyright law.  Dissemination of this information,
		reproduction, or use of this material, whether in whole or in part,
		is strictly forbidden unless prior permission is obtained in writing
		from a duly authorized officer of Balance Software Corporation.
		

	File:				asn1_oids.h

	Author:				Brian Doyle
	Date Created:		September 07, 2005
	Last Modified:		September 07, 2005

	Description:

	A very short (and incomplete) list of ANS.1 oid's

	For complete repository see:
	
		http://asn1.elibel.tm.fr/oid/index.htm

\*----------------------------------------------------------------------------------------*/
#ifndef __asn1_oids_h__
#define __asn1_oids_h__



// level 0:


// { }
enum asn1_oid_root {
	k_asn1_oid_ccitt								=		0			,
	k_asn1_oid_itu_r								=		0			,
	k_asn1_oid_itu_t								=		0			,
	
	k_asn1_oid_iso									=		1			,
	
	k_asn1_oid_joint_iso_ccitt						=		2			,
	k_asn1_oid_joint_iso_itu_t						=		2			,
};


// level 1:


// { iso(1) }
enum asn1_oid_iso {
	k_asn1_oid_standard								=		0			,
	k_asn1_oid_registration_authority				=		1			,
	k_asn1_oid_member_body							=		2			,
	k_asn1_oid_identified_organization				=		3			,
};


// { joint-iso-itu-t(2) }
enum asn1_oid_joint_iso_itu_t {
	k_asn1_oid_presentation							=		0			,
	k_asn1_oid_asn1									=		1			,
	k_asn1_oid_association_control					=		2			,
	k_asn1_oid_reliable_transfer					=		3			,
	k_asn1_oid_remote_operations					=		4			,
	k_asn1_oid_directory							=		5			,
	k_asn1_oid_ds									=		5			,
	k_asn1_oid_mhs									=		6			,
	k_asn1_oid_mhs_motis							=		6			,
	k_asn1_oid_ccr									=		7			,
	k_asn1_oid_oda									=		8			,
	k_asn1_oid_ms									=		9			,
	k_asn1_oid_osi_management						=		9			,
	k_asn1_oid_transaction_processing				=		10			,
	k_asn1_oid_distinguished_object_reference		=		11			,
	k_asn1_oid_dor									=		11			,
	k_asn1_oid_reference_data_transfer				=		12			,
	k_asn1_oid_network_layer						=		13			,
	k_asn1_oid_network_layer_management				=		13			,
	k_asn1_oid_transport_layer						=		14			,
	k_asn1_oid_transport_layer_management			=		14			,
	k_asn1_oid_datalink_layer						=		15			,
	k_asn1_oid_datalink_layer_management			=		15			,
	k_asn1_oid_country								=		16			,
	k_asn1_oid_registration_procedure				=		17			,
	k_asn1_oid_registration_procedures				=		17			,
	k_asn1_oid_physical_layer						=		18			,
	k_asn1_oid_physical_layer_management			=		18			,
	k_asn1_oid_mheg									=		19			,
	k_asn1_oid_generic_uls							=		20			,
	k_asn1_oid_generic_upper_layers_security		=		20			,
	k_asn1_oid_guls									=		20			,
	k_asn1_oid_transport_layer_security_protocol	=		21			,
	k_asn1_oid_network_layer_security_protocol		=		22			,
	k_asn1_oid_international_organizations			=		23			,
	k_asn1_oid_international_ra						=		23			,
	k_asn1_oid_sios									=		24			,
	k_asn1_oid_uuid									=		25			,
	k_asn1_oid_odp									=		26			,
	k_asn1_oid_upu									=		40			,
};


// level 2:

// { iso(1) member-body(2) }
enum asn1_oid_member_body {
	k_asn1_oid_au									=		36			,
	k_asn1_oid_at									=		40			,
	k_asn1_oid_austria								=		40			,
	k_asn1_oid_be									=		56			,
	k_asn1_oid_unassigned							=		110			,
	k_asn1_oid_cn									=		156			,
	k_asn1_oid_tw									=		158			,
	k_asn1_oid_taiwan								=		158			,
	k_asn1_oid_cz									=		203			,
	k_asn1_oid_fi									=		246			,
	k_asn1_oid_f									=		250			,
	k_asn1_oid_fr									=		250			,
	k_asn1_oid_de									=		276			,
	k_asn1_oid_ca									=		302			,
	k_asn1_oid_canada								=		302			,
	k_asn1_oid_hk									=		344			,
	k_asn1_oid_372		/* ireland */				=		372			,
	k_asn1_oid_jisc									=		392			,
	k_asn1_oid_jp									=		392			,
	k_asn1_oid_korea								=		410			,
	k_asn1_oid_nl									=		528			,
	k_asn1_oid_nni									=		528			,
	k_asn1_oid_no									=		578			,
	k_asn1_oid_pl									=		616			,
	k_asn1_oid_ru									=		643			,
	k_asn1_oid_sg									=		702			,
	k_asn1_oid_vn									=		704			,
	k_asn1_oid_se									=		752			,
	k_asn1_oid_gb									=		826			,
	k_asn1_oid_uk									=		826			,
	k_asn1_oid_us									=		840			,
	k_asn1_oid_usa									=		840			,
	k_asn1_oid_862		/* venezuela */				=		862			,
	k_asn_old_taiwan_								=		886			,
	k_asn1_oid_taiwan_illegal						=		886			,
};


// level 3:


// { joint-iso-itu-t(2) country(16) us(840) }
enum asn1_oid_us {
	k_asn1_oid_organization							=		1			,
	k_asn1_oid_sun									=		113536		,
	k_asn1_oid_rsadsi								=		113549		,
	k_asn1_oid_mit									=		113554		,
	k_asn1_oid_microsoft							=		113556		,
};	


// level 4:


// { joint-iso-itu-t(2) country(16) us(840) organization(1) }
enum asn1_oid_organization {
	k_asn1_oid_gov									=		101			,
};


// { joint-iso-itu-t(2) country(16) us(840) rsadsi(113549) }
enum asn1_oid_rsadsi {
	k_asn1_oid_pkcs									=		1			,
	k_asn1_oid_digest_algorithm						=		2			,
	k_asn1_oid_encryption_algorithm					=		3			,
};


// level 5:


// { joint-iso-itu-t(2) country(16) us(840) organization(1) gov(101) }
enum asn1_oid_gov {
	k_asn1_oid_csor									=		3			,
};


// { joint-iso-itu-t(2) country(16) us(840) rsadsi(113549) pkcs(1) }
enum asn1_oid_pkcs {
	k_asn1_oid_pkcs_1								=		1			,
	k_asn1_oid_bsafe								=		2			,
	k_asn1_oid_pkcs_3								=		3			,
	k_asn1_oid_pkcs_5								=		5			,
	k_asn1_oid_pkcs_7								=		7			,
	k_asn1_oid_pkcs_8								=		8			,
	k_asn1_oid_pkcs_9								=		9			,
	k_asn1_oid_pkcs_10								=		10			,
	k_asn1_oid_pkcs_12								=		12			,
	k_asn1_oid_pkcs_15								=		15			,
	k_asn1_oid_id_smime								=		16			,
};


// level 6:


// { joint-iso-itu-t(2) country(16) us(840) organization(1) gov(101) csor(3) }
enum asn1_oid_csor {
	k_asn1_oid_nist_algorithm						=		4			,
};


// { joint-iso-itu-t(2) country(16) us(840) rsadsi(113549) pkcs(1) pkcs-1(1) }
enum asn1_oid_pkcs_1 {
	k_asn1_oid_rsa_encryption						=		1			,
	k_asn1_oid_md2_with_rsa_encryption				=		2			,
	k_asn1_oid_md4_with_rsa_encryption				=		3			,
	k_asn1_oid_md5_with_rsa_encryption				=		4			,
	k_asn1_oid_sha1_with_rsa_encryption				=		5			,
	k_asn1_oid_sha1_with_rsa_signature				=		5			,
	k_asn1_oid_ripemd_160_with_rsa_encryption		=		6			,
	k_asn1_oid_rsa_oaep_encryption_set				=		6			,
	k_asn1_oid_id_rsaes_oaep						=		7			,
	k_asn1_oid_id_mgfl								=		8			,
	k_asn1_oid_id_p_specified						=		9			,
	k_asn1_oid_id_rsassa_pss						=		10			,
	k_asn1_oid_sha256_with_rsa_encryption			=		11			,
	k_asn1_oid_sha384_with_rsa_encryption			=		12			,
	k_asn1_oid_sha512_with_rsa_encryption			=		13			,
	k_asn1_oid_sha224_with_rsa_encryption			=		14			,
};


// level 7:

// { joint-iso-itu-t(2) country(16) us(840) organization(1) gov(101) csor(3)
//	 nistalgorithm(4) }
enum asn1_oid_nist_algorithm {
	k_asn1_oid_modules								=		0			,
};


// level 8

// { joint-iso-itu-t(2) country(16) us(840) organization(1) gov(101) csor(3)
//	 nistalgorithm(4) modules(0) }
enum asn1_oid_modules {
	k_asn1_oid_sha2									=		1			,
};


#endif // __asn1_oids_h__
