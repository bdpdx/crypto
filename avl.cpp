#include "queue.hpp"
#include "avl_node.hpp"


avl_node *avl_node::fetch( avl_node *in_root, void *in_key ) {
	int				result;

	for ( ;; ) {
		if ( in_root == nil ) return nil;
	
		result = compare_key( in_key, in_root->get_key() );
	
		if ( result < 0 ) in_root = in_root->m_left;
		else if ( result > 0 ) in_root = in_root->m_right;
		else return in_root;
	}
}


avl_result avl_node::insert( avl_node **in_root, avl_node *in_node ) {
	avl_result		tmp;
	int				result;
	
	if ( *in_root == nil ) { *in_root = in_node; return k_avl_balanced; }

	result = compare_key( in_node->get_key(), (*in_root)->get_key() );
	
	if ( result < 0 ) return ( tmp = insert( &(*in_root)->m_left, in_node ) ) == k_avl_balanced ? left_grown( in_root ) : tmp;
	if ( result > 0 ) return ( tmp = insert( &(*in_root)->m_right, in_node ) ) == k_avl_balanced ? right_grown( in_root ) : tmp;
	
	return k_avl_error;
}


avl_result avl_node::remove( avl_node **in_root, void *in_key, avl_node **out_node ) {
	int				result;
	avl_result		tmp = k_avl_balanced;

	if ( *in_root == nil ) return k_avl_no_error;
	
	result = compare_key( get_key(), (*in_root)->get_key() );
	
	if ( result < 0 ) return ( tmp = remove( &(*in_root)->m_left, in_key, out_node ) ) == k_avl_balanced ? left_shrunk( in_root ) : tmp;
	if ( result > 0 ) return ( tmp = remove( &(*in_root)->m_right, in_key, out_node ) ) == k_avl_balanced ? right_shrunk( in_root ) : tmp;
	
	if ( (*in_root)->m_left && find_highest( *in_root, &(*in_root)->m_left, out_node, &tmp ) ) {
		if ( tmp == k_avl_balanced ) left_shrunk( in_root );
		return tmp;
	}
	
	if ( (*in_root)->m_right && find_lowest( *in_root, &(*in_root)->m_right, out_node, &tmp ) ) {
		if ( tmp == k_avl_balanced ) tmp = right_shrunk( in_root );
		return tmp;
	}
	
	*out_node = *in_root;
	*in_root = nil;
	
	return k_avl_balanced;
}


avl_result avl_node::left_grown( avl_node **in_root ) {
	switch ( (*in_root)->m_skew ) {
		case k_left_skew: {
			if ( (*in_root)->m_left->m_skew == k_left_skew ) {
				(*in_root)->m_skew = (*in_root)->m_left->m_skew = k_no_skew;
				avl_rotate_right( in_root );
			} else {
				switch ( (*in_root)->m_left->m_right->m_skew ) {
					case k_left_skew: {
						(*in_root)->m_skew = k_right_skew;
						(*in_root)->m_left->m_skew = k_no_skew;
					} break;
					
					case k_right_skew: {
						(*in_root)->m_skew = k_no_skew;
						(*in_root)->m_left->m_skew = k_left_skew;
					} break;
					
					default: {
						(*in_root)->m_skew = k_no_skew;
						(*in_root)->m_left->m_skew = k_no_skew;
					} break;
				}
				(*in_root)->m_left->m_right->m_skew = k_no_skew;
				avl_rotate_left( &(*in_root)->m_left );
				avl_rotate_right( in_root );
			}
		} return k_avl_no_error;
		
		case k_right_skew: {
			(*in_root)->m_skew = k_no_skew;
		} return k_avl_no_error;
		
		default: {
			(*in_root)->m_skew = k_left_skew;
		} return k_avl_balanced;
	}
}


avl_result avl_node::right_grown( avl_node **in_root ) {
	switch ( (*in_root)->m_skew ) {
		case k_left_skew: {
			(*in_root)->m_skew = k_no_skew;
		} return k_avl_no_error;
		
		case k_right_skew: {
			if ( (*in_root)->m_right->m_skew == k_right_skew ) {
				(*in_root)->m_skew = (*in_root)->m_right->m_skew = k_no_skew;
				avl_rotate_left( in_root );
			} else {
				switch ( (*in_root)->m_right->m_left->m_skew ) {
					case k_left_skew: {
						(*in_root)->m_skew = k_no_skew;
						(*in_root)->m_right->m_skew = k_right_skew;
					} break;
					
					case k_right_skew: {
						(*in_root)->m_skew = k_left_skew;
						(*in_root)->m_right->m_skew = k_no_skew;
					} break;
					
					default: {
						(*in_root)->m_skew = k_no_skew;
						(*in_root)->m_right->m_skew = k_no_skew;
					} break;
				}
				(*in_root)->m_right->m_left->m_skew = k_no_skew;
				avl_rotate_right( &(*in_root)->m_right );
				avl_rotate_left( in_root );
			}
		} return k_avl_no_error;
		
		default: {
			(*in_root)->m_skew = k_right_skew;
		} return k_avl_balanced;
	}
}


avl_result avl_node::left_shrunk( avl_node **in_root ) {
	switch ( (*in_root)->m_skew ) {
		case k_left_skew: {
			(*in_root)->m_skew = k_no_skew;
		} return k_avl_balanced;
		
		case k_right_skew: {
			if ( (*in_root)->m_right->m_skew == k_right_skew ) {
				(*in_root)->m_skew = k_right_skew;
				(*in_root)->m_right->m_skew = k_left_skew;
				avl_rotate_left( in_root );
				return k_avl_balanced;
			} else if ( (*in_root)->m_right->m_skew == k_no_skew ) {
				(*in_root)->m_skew = k_right_skew;
				(*in_root)->m_right->m_skew = k_left_skew;
				avl_rotate_left( in_root );
				return k_avl_no_error;
			} else {
				switch ( (*in_root)->m_right->m_left->m_skew ) {
					case k_left_skew: {
						(*in_root)->m_skew = k_no_skew;
						(*in_root)->m_right->m_skew = k_right_skew;
					} break;
					
					case k_right_skew: {
						(*in_root)->m_skew = k_left_skew;
						(*in_root)->m_right->m_skew = k_no_skew;
					} break;
					
					default: {
						(*in_root)->m_skew = k_no_skew;
						(*in_root)->m_right->m_skew = k_no_skew;
					} break;
				}
				(*in_root)->m_right->m_left->m_skew = k_no_skew;
				avl_rotate_right( &(*in_root)->m_right );
				avl_rotate_left( in_root );
				return k_avl_balanced;
			}
		} // we've returned by now
		
		default: {
			(*in_root)->m_skew = k_right_skew;
		} return k_avl_no_error;
	}
}


avl_result avl_node::right_shrunk( avl_node **in_root ) {
	switch ( (*in_root)->m_skew ) {
		case k_left_skew: {
			if ( (*in_root)->m_left->m_skew == k_left_skew ) {
				(*in_root)->m_skew = (*in_root)->m_left->m_skew = k_no_skew;
				avl_rotate_right( in_root );
				return k_avl_balanced;
			} else if ( (*in_root)->m_left->m_skew == k_no_skew ) {
				(*in_root)->m_skew = k_left_skew;
				(*in_root)->m_left->m_skew = k_right_skew;
				avl_rotate_right( in_root );
				return k_avl_no_error;
			} else {
				switch ( (*in_root)->m_left->m_right->m_skew ) {
					case k_left_skew: {
						(*in_root)->m_skew = k_right_skew;
						(*in_root)->m_left->m_skew = k_no_skew;
					} break;
					
					case k_right_skew: {
						(*in_root)->m_skew = k_no_skew;
						(*in_root)->m_left->m_skew = k_left_skew;
					} break;
					
					default: {
						(*in_root)->m_skew = k_no_skew;
						(*in_root)->m_left->m_skew = k_no_skew;
					} break;
				}
				(*in_root)->m_left->m_right->m_skew = k_no_skew;
				avl_rotate_left( &(*in_root)->m_left );
				avl_rotate_right( in_root );
				return k_avl_balanced;
			}
		} // we've returned by now

		case k_right_skew: {
			(*in_root)->m_skew = k_no_skew;
		} return k_avl_balanced;
		
		default: {
			(*in_root)->m_skew = k_left_skew;
		} return k_avl_no_error;
	}
}


long avl_node::find_highest( avl_node *in_target, avl_node **in_root, avl_node **out_node, avl_result *out_result ) {
	avl_node	   *tmp;
	
	*out_result = k_avl_balanced;
	
	if ( *in_root == nil ) return k_node_not_replaced;

	if ( (*in_root)->m_right ) {
		if ( ! find_highest( in_target, &(*in_root)->m_right, out_node, out_result ) ) return k_node_not_replaced;
		if ( *out_result == k_avl_balanced ) *out_result = right_shrunk( in_root );
		return k_node_replaced;
	}

	*in_target = **in_root;

	tmp = *in_root;
	*in_root = (*in_root)->m_left;
	*out_node = tmp;

	return k_node_replaced;
}


long avl_node::find_lowest( avl_node *in_target, avl_node **in_root, avl_node **out_node, avl_result *out_result ) {
	avl_node	   *tmp;
	
	*out_result = k_avl_balanced;
	
	if ( *in_root == nil ) return k_node_not_replaced;

	if ( (*in_root)->m_left ) {
		if ( ! find_lowest( in_target, &(*in_root)->m_left, out_node, out_result ) ) return k_node_not_replaced;
		if ( *out_result == k_avl_balanced ) *out_result = left_shrunk( in_root );
		return k_node_replaced;
	}
	
	*in_target = **in_root;
	
	tmp = *in_root;
	*in_root = (*in_root)->m_right;
	*out_node = tmp;
	
	return k_node_replaced;
}


void avl_node::avl_rotate_left( avl_node **in_root ) {
	avl_node   *tmp = *in_root;
	
	*in_root = (*in_root)->m_right;
	tmp->m_right = (*in_root)->m_left;
	(*in_root)->m_left = tmp;
}


void avl_node::avl_rotate_right( avl_node **in_root ) {
	avl_node   *tmp = *in_root;
	
	*in_root = (*in_root)->m_left;
	tmp->m_left = (*in_root)->m_right;
	(*in_root)->m_right = tmp;
}


#if __MWERKS__
	#pragma warn_possunwant		off
#endif

int avl_node::traverse( avl_node *in_root, avl_traversal_callback in_callback, void *in_context, avl_traversal_method in_method, long in_depth ) {
	int			err;

	if ( in_root == nil ) return 0;
	
	switch ( in_method ) {
		case k_prefix: {
			err = (*in_callback)( in_root, in_context, in_depth );
			if ( ! err ) err = traverse( in_root->m_left, in_callback, in_context, in_method, in_depth + 1 );
			if ( ! err ) err = traverse( in_root->m_right, in_callback, in_context, in_method, in_depth + 1 );
		} break;

		case k_infix: {
			err = traverse( in_root->m_left, in_callback, in_context, in_method, in_depth + 1 );
			if ( ! err ) err = (*in_callback)( in_root, in_context, in_depth );
			if ( ! err ) err = traverse( in_root->m_right, in_callback, in_context, in_method, in_depth + 1 );
		} break;

		case k_postfix: {
			err = traverse( in_root->m_left, in_callback, in_context, in_method, in_depth + 1 );
			if ( ! err ) err = traverse( in_root->m_right, in_callback, in_context, in_method, in_depth + 1 );		
			if ( ! err ) err = (*in_callback)( in_root, in_context, in_depth );
		} break;
		
		case k_breadth_first: {
			queue<avl_node>		q;
			unsigned long		seen = 0, next = 1;

			while ( ! ( err = (*in_callback)( in_root, in_context, in_depth ) ) ) {
				if ( in_root->m_left ) q.insert( in_root->m_left );
				if ( in_root->m_right ) q.insert( in_root->m_right );
				if ( ! ( in_root = q.remove() ) ) break;
				if ( ++seen == next ) { ++in_depth; next = ( next + 1 ) * 2 - 1; }
			}
		} break;
		
		default:		err = 0;			break;
	}

	return err;
}
