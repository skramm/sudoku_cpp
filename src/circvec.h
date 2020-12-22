/**
\file circvec.h
\brief holds a class template designed to handle a vector as circular

\todo TESTING NEEDED !!!
*/

template<typename T>
class CircVec
{
	private:
		std::vector<T> _data;
	public:
		std::vector<T>&       data()       { return _data; }
		const std::vector<T>& data() const { return _data; }
		size_t size() const { return _data.size(); }
		void AddElem( const T& elem )
		{
			_data.push_back( elem );
		}
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
#if 0
		std::iterator<T> begin()
		{
			return std::begin( _data );
		}
		auto end()
		{
			return std::end( _data );
		}
#endif

};

