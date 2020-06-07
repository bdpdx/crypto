#ifndef __avl_node_hpp__
#define __avl_node_hpp__


#ifndef nil
	#define nil			0
#endif


enum avl_skew { k_no_skew , k_left_skew , k_right_skew };
enum avl_replaced { k_node_not_replaced, k_node_replaced };
enum avl_result { k_avl_no_error, k_avl_error, k_avl_balanced };
enum avl_traversal_method { k_prefix, k_infix, k_postfix, k_breadth_first };


typedef int (*avl_traversal_callback)( void *in_avl_node, void *in_context, long in_depth );


class avl_node {

public:

	avl_node() { m_left = m_right = nil; m_skew = k_no_skew; }
	virtual ~avl_node() { }

	avl_node *fetch( avl_node *in_root, void *in_key );
	
	avl_result insert( avl_node **in_root, avl_node *in_node );
	avl_result remove( avl_node **in_root, void *in_key, avl_node **out_node );

	int traverse( avl_node *in_root, avl_traversal_callback in_callback, void *in_context = nil, avl_traversal_method in_method = k_prefix ) { return traverse( in_root, in_callback, in_context, in_method, 0 ); }
	
protected:

	// compare_key() must return a value that is to zero as in_lhs is to in_rhs
	virtual int compare_key( void *in_lhs, void *in_rhs ) = 0;

	// get_key() must return the key that this node contains (i.e. returns m_key)
	virtual void *get_key() = 0;
	
//	void			   *m_key;			// subclasses must declare this

private:

	void avl_rotate_left( avl_node **in_root );
	void avl_rotate_right( avl_node **in_root );

	avl_result left_grown( avl_node **in_root );
	avl_result right_grown( avl_node **in_root );
	
	avl_result left_shrunk( avl_node **in_root );
	avl_result right_shrunk( avl_node **in_root );
		
	long find_lowest( avl_node *in_target, avl_node **in_root, avl_node **out_node, avl_result *out_result );
	long find_highest( avl_node *in_target, avl_node **in_root, avl_node **out_node, avl_result *out_result );

	int traverse( avl_node *in_root, avl_traversal_callback in_callback, void *in_context, avl_traversal_method in_method, long in_depth );
	
	avl_node		   *m_left;
	avl_node		   *m_right;
	
	avl_skew			m_skew;

};



#endif // __avl_node_hpp__
