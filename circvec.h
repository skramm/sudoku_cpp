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
		int _current_pos=0;
	public:
		size_t size() const { return _data.size(); }
		void AddElem( const T& elem ) { _data.push_back( elem );	}
		const T& GetElem( size_t idx ) const
		{
			if( idx +_current_pos >= size() )
				return _data[ (idx + _current_pos) % size() ];
			return _data[idx + _current_pos];
		}
		T& GetElem( size_t idx )
		{
			if( idx +_current_pos >= size() )
				return _data[ (idx + _current_pos) % size() ];
			return _data[idx + _current_pos];
		}
		void RotateDown( size_t n )
		{
			assert( n<size() );
			int p = _current_pos - n;
			if( p < 0 )
				_current_pos = size() - n;
			else
				_current_pos = p;
		}
};

