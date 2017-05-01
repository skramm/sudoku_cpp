/**
\file circvec.hpp
\brief holds a class template designed to handle a vector as circular

\todo TESTING NEEDED !!!
*/

template<typename T>
class circvec
{
	private:
		std::vector<T> _data;
//		int _current_pos=0;
	public:
		std::vector<T>&       data()       { return _data; }
		const std::vector<T>& data() const { return _data; }
		size_t size() const { return _data.size(); }
		void AddElem( const T& elem ) { _data.push_back( elem );	}
		const T& GetElem( size_t idx ) const
		{
			if( idx >= size() )
				return _data[ (idx ) % size() ];
			return _data[idx];
		}
		T& GetElem( size_t idx )
		{
			if( idx >= size() )
				return _data[ (idx) % size() ];
			return _data[idx];
		}
/*		void RotateDown( size_t n )
		{
			assert( n<size() );
			int p = _current_pos - n;
			if( p < 0 )
				_current_pos = size() - n;
			else
				_current_pos = p;
		}*/
};

