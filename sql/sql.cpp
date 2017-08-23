#include "sql.h"

sql::sql(const std::string& _user,\
		 const std::string& _passwd,\
		 const std::string& _ip,\
		 const std::string& _db,\
		 const int _port)
	:user(_user),\
	passwd(_passwd),ip(_ip),db(_db),port(_port)
{
	conn = mysql_init(NULL);
	if(conn == NULL)
	{
		std::cout <<"sql_init error!"<<std::endl;
	}
}

int sql::connect()
{
	if(mysql_real_connect(conn,"127.0.0.1",user.c_str(),passwd.c_str(),db.c_str(),port,NULL,0))
	{
		std::cout <<"connect success!"<< std::endl;
		return 0;
	}
	else
	{
		std::cout<<"connect fail"<<std::endl;
		return -1;
	}
}

int sql::insert(const std::string& name,\
			   const std::string& sex,\
			   const std::string& school)
{	
	std::string insert_sql = "INSERT INTO student_info (name,sex,school) VALUES ('";
	insert_sql+=name;
	insert_sql+="','";
	insert_sql+=sex;
	insert_sql+="','";
	insert_sql+=school;
	insert_sql+="')";

	//发送语句,成功返回0
	int ret = mysql_query(conn,insert_sql.c_str());
	if(ret)
		std::cout<<"insert fail!"<<std::endl;
	else
		std::cout<<"insert success!"<<std::endl;
	return ret;
}
int sql::select()
{
	std::string select_sql = "select * from student_info";
	int ret = mysql_query(conn,select_sql.c_str());
	if(ret == 0)
	{
		//获取执行结果
		MYSQL_RES *res = mysql_store_result(conn);
		if(res)
		{
			//获取行数
			int lines = mysql_num_rows(res);
			//获取列数
			int cols = mysql_num_fields(res);
		//	std::cout << "lines: "<<lines<<" cols: "<<cols<<std::endl;
			
			//获取结果信息
			//mysql_fetch_field获取列名
			MYSQL_FIELD *fd = NULL;
			for(; fd = mysql_fetch_field(res);)
			{
				std::cout<<fd->name<< "  ";
			}
			std::cout <<std::endl;
			int i = 0;
			int j = 0;
			for(; i < lines; ++i)
			{
				//相当于 返回一个二维数组char**
				MYSQL_ROW row = mysql_fetch_row(res);
				j = 0;
				for(; j < cols;++j)
				{
					std::cout<<row[j]<<" ";
				}
				std::cout<<std::endl;
			}
		}
		else
			return -1;
	}
	else
		return -2;
	return 0;
}
sql::~sql()
{
	mysql_close(conn);
}
