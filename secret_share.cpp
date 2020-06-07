#include "secret_share.h"


#if ENABLE_SECRET_SHARE_GENERATORS
#include "apn_utilities.h"


create_shadows_params::create_shadows_params() {
	in_num_bits_in_prime = 0;
	in_num_shadows_to_create = 0;
	in_num_shadows_to_reconstruct_secret = 0;
	in_random_number_file = "/dev/urandom";
	in_rounds = k_default_rabin_miller_rounds;
	in_secret = nil;

	out_shadows = nil;
}


void create_shadows( create_shadows_params *in_params ) {
	apn				   *coefficients = nil, d, minus_one = -1;
	__s32				i, j, k, n, o;

	k = in_params->in_num_shadows_to_reconstruct_secret;
	n = in_params->in_num_shadows_to_create;
	o = n - 1;
	
	// choose a large prime p which is both larger than the secret and larger than the largest possible shadow
	if ( ! in_params->io_prime ) {
		for ( ;; ++in_params->in_num_bits_in_prime ) {
			in_params->io_prime = generate_prime( in_params->in_num_bits_in_prime, in_params->in_random_number_file, in_params->in_rounds );
			if ( in_params->io_prime > *in_params->in_secret && in_params->io_prime > in_params->in_num_shadows_to_create ) break;
		}
	} else {
		if ( ! ( in_params->io_prime > *in_params->in_secret && in_params->io_prime > in_params->in_num_shadows_to_create ) ) _throw( err_bad_parameter );
	}
	
	in_params->out_shadows = new shamir_shadow[ n ];
	
	_try {
		coefficients = new apn[ o ];

		for ( i = 0; i < o; ++i ) {
			coefficients[ i ] = random_apn( in_params->in_random_number_file, minus_one, in_params->io_prime );
		}

		// loop over the number of shadows to create
		for ( i = 0; i < n; ++i ) {
			in_params->out_shadows[ i ].x = i + 1;
			in_params->out_shadows[ i ].y = *in_params->in_secret;
			
			for ( j = 1; j < k; ++j ) {
				in_params->out_shadows[ i ].y += x_to_the_n_mod_m( in_params->out_shadows[ i ].x, j, in_params->io_prime ) * coefficients[ j - 1 ];
				in_params->out_shadows[ i ].y %= in_params->io_prime;
			}
		}
	} _catch
	
	_if_err _delete_( in_params->out_shadows );
	
	if ( coefficients ) {
		for ( i = 0; i < o; ++i ) coefficients[ i ].clear_and_zero();
		
		delete[] coefficients;
	}
	
	_return;
}
#endif // ENABLE_SECRET_SHARE_GENERATORS


const apn recover_secret( shamir_shadow *in_shadows, __u32 in_num_shadows, const apn &in_prime ) {
	// lagrange polynomial interpolation with no floating point operations

	__u32			i, j;
	apn				tn, td, rn, rd( 1 );
	
	for ( i = 0; i < in_num_shadows; ++i ) {
		for ( j = 0, tn = td = 1; j < in_num_shadows; ++j ) {
			if ( i != j ) {
				tn *= -in_shadows[ j ].x;
				td *= in_shadows[ i ].x - in_shadows[ j ].x;
			}
		}
		
		tn *= in_shadows[ i ].y;
		
		tn *= rd;
		rn *= td;
		rd *= td;
		
		rn += tn;
	}

	tn = rn / rd % in_prime;

	return apn( tn.is_negative() ? tn + in_prime : tn );
}
